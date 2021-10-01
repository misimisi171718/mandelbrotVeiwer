#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <iostream>
#include <fstream>

#include "opencl.hpp"

cl::Platform platform;
cl::Program program;
cl::Context context;
cl::Device device;
cl::CommandQueue queue;
cl::Kernel kernel;

void drawMandelbrot(
        SDL_Texture* text,
        const std::complex<double>& center,
        const std::complex<double>& start,
        const double& zoom,
        const bool julia)
{
    //math setup
    int winW, winH;
    SDL_QueryTexture(text, NULL, NULL, &winW, &winH);
    double winWD = static_cast<double>(winW);
    double winHD = static_cast<double>(winH);
    double aspect = winWD / winHD;
    cl_float2 step;
    cl_float2 offset;
    step.s[0] = static_cast<float>(1.0 / (zoom *          winWD));
    step.s[1] = static_cast<float>(1.0 / (zoom * aspect * winHD));
    offset.s[0] = static_cast<float>(center.real() - (1 / (zoom * 2)));
    offset.s[1] = static_cast<float>(center.imag() - (1 / (zoom * 2 * aspect)));

    //opencl execution
    cl::Buffer bufer(
        context,
        CL_MEM_WRITE_ONLY|CL_MEM_HOST_READ_ONLY,
        sizeof(cl_double3)*winW*winH);
    
    cl_float2 startFloat = {static_cast<float>(start.real()),static_cast<float>(start.imag())};

    kernel.setArg(0,bufer);
    kernel.setArg(1,offset);
    kernel.setArg(2,step);
    kernel.setArg(3,startFloat);
    kernel.setArg(4,500);
    kernel.setArg(5,julia ? 1 : 0);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(winW,winH));

    auto startTime = std::chrono::high_resolution_clock::now();
    queue.finish();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end-startTime).count() << "\r";
    std::cout.flush();

    cl_uchar3* pixels = new cl_uchar3[winH*winW];

    queue.enqueueReadBuffer(bufer, CL_TRUE, 0, sizeof(cl_uchar3)*winW*winH, pixels);
    SDL_UpdateTexture(text, NULL, pixels, winW*sizeof(cl_uchar3));

    delete[] pixels;
}

void openclInit()
{
    cl_int err;
    //get opencl platforms
    {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if(platforms.size() == 0)
        {
            std::cout << "No OpenCL platform found.\nPleas check your dirivers.\n";
            std::exit(-1);
        }
        else if(platforms.size() == 1)
        {
            std::string x;
            platforms[0].getInfo(CL_PLATFORM_NAME,&x);
            std::cout << "One OpenCL pleatform was found: " << x << "\n";
            platform = platforms[0];
        }
        else
        {
            std::cout << "Multiple OpenCL divices were found pleas select one:\n";
            for (size_t i = 0; i < platforms.size(); i++)
            {
                std::string x;
                platforms[i].getInfo(CL_PLATFORM_NAME,&x);
                std::cout << " " << i << ") " << x << "\n";
            }
            int x = 0;
            std::cin >> x;
            platform = platforms[x];
        }
    }
    //get opencl devices
    {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL,&devices);
        if(devices.size() == 0)
        {
            std::cout << "No OpenCL devices found.\nPleas check your dirivers.\n";
            std::exit(-1);
        }
        else if(devices.size() == 1)
        {
            std::string vendor,version;
            devices[0].getInfo(CL_DEVICE_VENDOR,&vendor);
            devices[0].getInfo(CL_DEVICE_VERSION,&version);
            std::cout << "One OpenCL device was found: " << vendor << " " << version << "\n";
            device = devices[0];
        }
        else
        {
            std::cout << "Multiple OpenCL devices were found pleas select one:\n";
            for (size_t i = 0; i < devices.size(); i++)
            {
                std::string x,y;
                devices[i].getInfo(CL_DEVICE_VENDOR,&x);
                devices[i].getInfo(CL_DEVICE_VERSION,&y);
                std::cout << " " << i << ") " << x << " " << y <<"\n";
            }
            int x = 0;
            std::cin >> x;
            device = devices[x];
        }
        std::string x;
        device.getInfo(CL_DEVICE_VERSION,&x);
        //TODO: opencl verson check
    }
    //load kernel file
    cl::Program::Sources source;
    {
        std::ifstream file("mandelbrot.cl");
        if(file)
        {
            std::string temp;
            temp.assign(
                std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>()
            );
            
            source = cl::Program::Sources(1,{temp.c_str(),temp.length() + 1});
        }
        else
        {
            std::cout << "File not found.\n";
            std::exit(-1);
        }
        
    }
    //create context and program 
    context = cl::Context(device,NULL,NULL,NULL,&err);
    program = cl::Program(context,source,&err);
    //compile kernel
    {
        auto error = program.build("-cl-std=CL1.2 -Werror");
        if(error != CL_BUILD_SUCCESS)
        {
            std::cout << "kernel build faild\nkelnel build log:\n";
            std::cout <<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
            std::exit(-1);
        }
    }

    kernel = cl::Kernel(program,"mandelbrrot",&err);
    queue = cl::CommandQueue(context,device,0UL,&err);
}