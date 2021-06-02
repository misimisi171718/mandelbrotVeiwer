cmake_minimum_required(VERSION 3.10.0)
project(mandelbrotVeiwer)

find_package(SDL2 REQUIRED)
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB srcFiles ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(mandelbrot ${srcFiles})
target_include_directories(mandelbrot PRIVATE /opt/cuda/targets/x86_64-linux/include) # todo do this properly
target_link_libraries(mandelbrot PRIVATE OpenGL OpenCL SDL2)