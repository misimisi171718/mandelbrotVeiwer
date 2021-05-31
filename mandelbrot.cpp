#include <iostream>
#include <fstream>
#include <chrono>
#include <math.h>
#include <SDL2/SDL.h>
#include <CL/cl.hpp>

#define WINDOW_WIDTH 700

cl_uchar3* pixels;
cl::Platform platform;
cl::Program program;
cl::Context context;
cl::Device device;
cl::CommandQueue queue;
cl::Kernel kernel;

void clError(cl_int err,int lineNumber = 0)
{
    if(err != 0)
    {
        std::string str;
        switch(err){
            // run-time and JIT compiler errors
            case 0: str = "CL_SUCCESS"; break;
            case -1: str = "CL_DEVICE_NOT_FOUND"; break;
            case -2: str = "CL_DEVICE_NOT_AVAILABLE"; break;
            case -3: str = "CL_COMPILER_NOT_AVAILABLE"; break;
            case -4: str = "CL_MEM_OBJECT_ALLOCATION_FAILURE"; break;
            case -5: str = "CL_OUT_OF_RESOURCES"; break;
            case -6: str = "CL_OUT_OF_HOST_MEMORY"; break;
            case -7: str = "CL_PROFILING_INFO_NOT_AVAILABLE"; break;
            case -8: str = "CL_MEM_COPY_OVERLAP"; break;
            case -9: str = "CL_IMAGE_FORMAT_MISMATCH"; break;
            case -10: str = "CL_IMAGE_FORMAT_NOT_SUPPORTED"; break;
            case -11: str = "CL_BUILD_PROGRAM_FAILURE"; break;
            case -12: str = "CL_MAP_FAILURE"; break;
            case -13: str = "CL_MISALIGNED_SUB_BUFFER_OFFSET"; break;
            case -14: str = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST"; break;
            case -15: str = "CL_COMPILE_PROGRAM_FAILURE"; break;
            case -16: str = "CL_LINKER_NOT_AVAILABLE"; break;
            case -17: str = "CL_LINK_PROGRAM_FAILURE"; break;
            case -18: str = "CL_DEVICE_PARTITION_FAILED"; break;
            case -19: str = "CL_KERNEL_ARG_INFO_NOT_AVAILABLE"; break;
            // compile-time errors
            case -30: str = "CL_INVALID_VALUE"; break;
            case -31: str = "CL_INVALID_DEVICE_TYPE"; break;
            case -32: str = "CL_INVALID_PLATFORM"; break;
            case -33: str = "CL_INVALID_DEVICE"; break;
            case -34: str = "CL_INVALID_CONTEXT"; break;
            case -35: str = "CL_INVALID_QUEUE_PROPERTIES"; break;
            case -36: str = "CL_INVALID_COMMAND_QUEUE"; break;
            case -37: str = "CL_INVALID_HOST_PTR"; break;
            case -38: str = "CL_INVALID_MEM_OBJECT"; break;
            case -39: str = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"; break;
            case -40: str = "CL_INVALID_IMAGE_SIZE"; break;
            case -41: str = "CL_INVALID_SAMPLER"; break;
            case -42: str = "CL_INVALID_BINARY"; break;
            case -43: str = "CL_INVALID_BUILD_OPTIONS"; break;
            case -44: str = "CL_INVALID_PROGRAM"; break;
            case -45: str = "CL_INVALID_PROGRAM_EXECUTABLE"; break;
            case -46: str = "CL_INVALID_KERNEL_NAME"; break;
            case -47: str = "CL_INVALID_KERNEL_DEFINITION"; break;
            case -48: str = "CL_INVALID_KERNEL"; break;
            case -49: str = "CL_INVALID_ARG_INDEX"; break;
            case -50: str = "CL_INVALID_ARG_VALUE"; break;
            case -51: str = "CL_INVALID_ARG_SIZE"; break;
            case -52: str = "CL_INVALID_KERNEL_ARGS"; break;
            case -53: str = "CL_INVALID_WORK_DIMENSION"; break;
            case -54: str = "CL_INVALID_WORK_GROUP_SIZE"; break;
            case -55: str = "CL_INVALID_WORK_ITEM_SIZE"; break;
            case -56: str = "CL_INVALID_GLOBAL_OFFSET"; break;
            case -57: str = "CL_INVALID_EVENT_WAIT_LIST"; break;
            case -58: str = "CL_INVALID_EVENT"; break;
            case -59: str = "CL_INVALID_OPERATION"; break;
            case -60: str = "CL_INVALID_GL_OBJECT"; break;
            case -61: str = "CL_INVALID_BUFFER_SIZE"; break;
            case -62: str = "CL_INVALID_MIP_LEVEL"; break;
            case -63: str = "CL_INVALID_GLOBAL_WORK_SIZE"; break;
            case -64: str = "CL_INVALID_PROPERTY"; break;
            case -65: str = "CL_INVALID_IMAGE_DESCRIPTOR"; break;
            case -66: str = "CL_INVALID_COMPILER_OPTIONS"; break;
            case -67: str = "CL_INVALID_LINKER_OPTIONS"; break;
            case -68: str = "CL_INVALID_DEVICE_PARTITION_COUNT"; break;

            // extension errors
            case -1000: str = "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR"; break;
            case -1001: str = "CL_PLATFORM_NOT_FOUND_KHR"; break;
            case -1002: str = "CL_INVALID_D3D10_DEVICE_KHR"; break;
            case -1003: str = "CL_INVALID_D3D10_RESOURCE_KHR"; break;
            case -1004: str = "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR"; break;
            case -1005: str = "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR"; break;
            default: str = "Unknown OpenCL error"; break;
        }
        std::cout << "error:" << str << " at line: " << lineNumber << "\n";
        std::exit(-1);
    }
}

#define CLC(x) clError(x,__LINE__)

struct complexNumber
{
    double real, imagenary;
    void operator*=(const complexNumber num)
    {
        double temp = real;
        real = (num.real * real) - (num.imagenary * imagenary);
        imagenary = (num.imagenary * temp) + (num.real * imagenary);
    }
    void operator/=(const complexNumber num)
    {
        double x = (num.imagenary*num.imagenary)+(num.real*num.real);
        real = ((num.real * real) + (num.imagenary * imagenary))/x;
        imagenary = ((num.imagenary * real) - (num.real * imagenary))/x;
    }
    void operator+=(const complexNumber num)
    {
        real = real + num.real;
        imagenary = imagenary + num.imagenary;
    }
    void operator-=(const complexNumber num)
    {
        real -= num.real;
        imagenary -= num.imagenary;
    }
};


std::ostream& operator<<(std::ostream& st, const complexNumber& num)
{
    st << num.real << " " << num.imagenary;
    return st;
}

void drawMandelbrot(
        SDL_Texture* text,
        const complexNumber& center,
        const double& zoom)
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
    offset.s[0] = static_cast<float>(center.real      - (1 / (zoom * 2)));
    offset.s[1] = static_cast<float>(center.imagenary - (1 / (zoom * 2 * aspect)));

    //opencl execution
    cl::Buffer bufer(
        context,
        CL_MEM_WRITE_ONLY|CL_MEM_HOST_READ_ONLY,
        sizeof(cl_double3)*winW*winH);
    
    CLC(kernel.setArg(0,bufer));
    CLC(kernel.setArg(1,offset));
    CLC(kernel.setArg(2,step));
    CLC(kernel.setArg(3,cl_float2{0.0,0.0}));
    CLC(kernel.setArg(4,500));
    CLC(kernel.setArg(5,0));
    CLC(queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(winW,winH)));

    auto start = std::chrono::high_resolution_clock::now();
    CLC(queue.finish());
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "\r";
    std::cout.flush();

    CLC(queue.enqueueReadBuffer(bufer, CL_TRUE, 0, sizeof(cl_uchar3)*winW*winH, pixels));

    SDL_UpdateTexture(text, NULL, pixels, winW*sizeof(cl_uchar3));
}

void updateWindow(SDL_Renderer* rend, SDL_Texture* text)
{
    SDL_RenderCopy(rend, text, NULL, NULL);
    SDL_RenderPresent(rend);
}

void resizeWindow(SDL_Renderer* rend, SDL_Texture*& text, SDL_Window* win)
{
    SDL_DestroyTexture(text);
    int w, h;
    SDL_GetWindowSize(win, &w, &h);
    text = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, w, h);
    delete[] pixels;
    pixels = new cl_uchar3[w*h];
}

complexNumber calcNewCenter(
    double mousex,double mousey,
    double winW,double winH,
    double zoom)
{
    complexNumber ret;
    ret.real      =  ((mousex / winW) - 0.5) / (zoom                );
    ret.imagenary =  ((mousey / winH) - 0.5) / (zoom * (winW / winH));
    return ret;
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
                platforms[0].getInfo(CL_PLATFORM_NAME,&x);
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
            std::string x,y,z;
            devices[0].getInfo(CL_DEVICE_VENDOR,&x);
            devices[0].getInfo(CL_DEVICE_VERSION,&y);
            std::cout << "One OpenCL device was found: " << x << " " << y << "\n";
            device = devices[0];
        }
        else
        {
            std::cout << "Multiple OpenCL devices were found pleas select one:\n";
            for (size_t i = 0; i < devices.size(); i++)
            {
                std::string x,y;
                devices[0].getInfo(CL_PLATFORM_NAME,&x);
                devices[0].getInfo(CL_DEVICE_VERSION,&y);
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
    std::string temp;
    {
        std::ifstream file("mandelbrot.cl");
        if(file)
        {
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
    clError(err,__LINE__);
    program = cl::Program(context,source,&err);
    clError(err);
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
    clError(err, __LINE__);
    queue = cl::CommandQueue(context,device,0UL,&err);
    clError(err, __LINE__);
}

int main()
{
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *texture;
    openclInit();

    pixels = new cl_uchar3[WINDOW_WIDTH*WINDOW_WIDTH];

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, SDL_WINDOW_RESIZABLE, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH, WINDOW_WIDTH);
    complexNumber center = { 0, 0};
    double zoom = 0.25;
    bool open = true;
    
    while (open) {
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                open = false;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    resizeWindow(renderer, texture, window);
                    drawMandelbrot(texture,center,zoom);
                    updateWindow(renderer,texture);
                    break;
                case SDL_WINDOWEVENT_MOVED:
                    updateWindow(renderer,texture);
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    updateWindow(renderer, texture);
                    break;
                default:
                    break;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y < 0)
                {
                    int winW, winH;
                    SDL_QueryTexture(texture, NULL, NULL, &winW, &winH);
                    int x,y;
                    SDL_GetMouseState(&x,&y);
                    center += calcNewCenter(static_cast<double>(x),static_cast<double>(y),static_cast<double>(winW),static_cast<double>(winH),static_cast<double>(zoom));
                    zoom = zoom * 1.1;
                    drawMandelbrot(texture,center,zoom);
                    updateWindow(renderer,texture);
                    break;
                }else if(event.wheel.y > 0){
                    int winW, winH;
                    SDL_QueryTexture(texture, NULL, NULL, &winW, &winH);
                    int x,y;
                    SDL_GetMouseState(&x,&y);
                    center -= calcNewCenter(static_cast<double>(x),static_cast<double>(y),static_cast<double>(winW),static_cast<double>(winH),static_cast<double>(zoom));
                    zoom = zoom / 1.1;
                    drawMandelbrot(texture,center,zoom);
                    updateWindow(renderer,texture);
                    break;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_r:
                    zoom = 0.25;
                    center = {0,0};
                    drawMandelbrot(texture,center,zoom);
                    updateWindow(renderer,texture);
                    break;
                case SDLK_w:
                    center.imagenary -= 1/(zoom*2);
                    drawMandelbrot(texture,center,zoom);
                    updateWindow(renderer,texture);
                    break;
                case SDLK_s:
                    center.imagenary += 1/(zoom*2);
                    drawMandelbrot(texture,center,zoom);
                    updateWindow(renderer,texture);
                    break;
                case SDLK_a:
                    center.real -= 1/(zoom*2);
                    drawMandelbrot(texture,center,zoom);
                    updateWindow(renderer,texture);
                    break;
                case SDLK_d:
                    center.real += 1/(zoom*2);
                    drawMandelbrot(texture,center,zoom);
                    updateWindow(renderer,texture);
                    break;
                case SDLK_q:
                    open = 0;
                    break;
                break;
            }
            
            }
        }
        SDL_Delay(100);
    }
    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
