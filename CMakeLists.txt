cmake_minimum_required(VERSION 3.10.0)
project(mandelbrotVeiwer)

add_subdirectory(lib)

file(GLOB srcFiles ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(mandelbrot ${srcFiles})
target_link_libraries(mandelbrot PRIVATE OpenCL SDL2 SDL2_ttf)
target_include_directories(mandelbrot PRIVATE lib/SDL_ttf)
add_custom_command(TARGET mandelbrot POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory
					${CMAKE_SOURCE_DIR}/src/resources $<TARGET_FILE_DIR:mandelbrot>)