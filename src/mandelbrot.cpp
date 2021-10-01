#include <iostream>
#include <fstream>
#include <chrono>
#include <math.h>
#include <SDL2/SDL.h>
#include <complex>
#include <array>

#include "opencl.hpp"
#include "button.hpp"

#define WINDOW_WIDTH 700

enum windowUPdateState{
    noUpdate = 0,
    update = 1,
    reDraw = 2,
    reSize = 3
};

using namespace std::complex_literals;

std::string locationString(const std::complex<double> Z, const std::complex<double> C)
{
    std::string ret = "Z: ";
    ret += std::to_string(Z.real()) + " + " + std::to_string(Z.imag()) + "i ";
    ret += "C: ";
    ret += std::to_string(C.real()) + " + " + std::to_string(C.imag()) + "i ";
    return ret;
}

int main()
{
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *texture;
    openclInit();


    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, SDL_WINDOW_RESIZABLE, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH, WINDOW_WIDTH);
    std::complex<double> center = { 0, 0};
    std::complex<double> start = { 0, 0};
    double zoom = 0.25;
    bool open = true;
    std::array<int,2> mouseMove{0,0};
    bool julia = false;
    windowUPdateState update = windowUPdateState::reSize;

    button locationInfo{renderer, locationString(center,start)};
    locationInfo.setPos({5,5});

    button juliaTogle{renderer, "julia/mandelbrot" };
    juliaTogle.setPos({5,50});
    juliaTogle.onClick([&julia](){
        julia = !julia;
    });

    button reset{renderer, "reset"};
    reset.setPos({5,90});
    reset.onClick([&](){
        zoom = 0.25;
        center = {0,0};
        start = {0,0};
    });

    button zoomInfo{renderer, std::to_string(zoom)};
    zoomInfo.setPos({5,130});

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
                    update = std::max(windowUPdateState::reSize,update);
                    break;
                case SDL_WINDOWEVENT_MOVED:
                case SDL_WINDOWEVENT_EXPOSED:
                    update = std::max(windowUPdateState::update,update);
                    break;
                default:
                    break;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y < 0)
                {
                    zoom = zoom * 1.1;
                    update = std::max(windowUPdateState::reDraw,update);
                    break;
                }else if(event.wheel.y > 0){
                    zoom = zoom / 1.1;
                    update = std::max(windowUPdateState::reDraw,update);
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                mouseMove[0] += event.motion.xrel;
                mouseMove[1] += event.motion.yrel;
                break;
            case SDL_MOUSEBUTTONDOWN:
                juliaTogle.mouseEvent(event.button);
                reset.mouseEvent(event.button);
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_r:
                    zoom = 0.25;
                    center = {0,0};
                    start = {0,0};
                    update = std::max(windowUPdateState::reDraw,update);
                    break;
                case SDLK_w:
                    center -= 1/(zoom*2) * 1.0i;
                    update = std::max(windowUPdateState::reDraw,update);
                    break;
                case SDLK_s:
                    center += 1/(zoom*2) * 1.0i;
                    update = std::max(windowUPdateState::reDraw,update);
                    break;
                case SDLK_a:
                    center -= 1/(zoom*2);
                    update = std::max(windowUPdateState::reDraw,update);
                    break;
                case SDLK_d:
                    center += 1/(zoom*2);
                    update = std::max(windowUPdateState::reDraw,update);
                    break;
                case SDLK_q:
                    open = 0;
                    break;
                case SDLK_e:
                    julia = !julia;
                    update = std::max(windowUPdateState::reDraw,update);
                    break;
            }
            }
        }
        bool shift = SDL_GetModState() & KMOD_SHIFT;
        auto mouseState = SDL_GetMouseState(nullptr,nullptr);
        if(shift ? mouseState & SDL_BUTTON_RMASK : mouseState & SDL_BUTTON_LMASK)
        {
            int winH,winW;
            SDL_GetWindowSize(window,&winW,&winH);
            center -= mouseMove[1]/std::abs(zoom*winH) * 1.0i;
            center -= mouseMove[0]/std::abs(zoom*winW);
            update = windowUPdateState::reDraw;
        }
        if(!shift ? mouseState & SDL_BUTTON_RMASK : mouseState & SDL_BUTTON_LMASK)
        {
            int winH,winW;
            SDL_GetWindowSize(window,&winW,&winH);
            start -= mouseMove[1]/std::abs(zoom*winH) * 1.0i;
            start -= mouseMove[0]/std::abs(zoom*winW);
            update = windowUPdateState::reDraw;
        }


        if(update >= windowUPdateState::reSize)
        {
            SDL_DestroyTexture(texture);
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, w, h);
        }
        if(update >= windowUPdateState::reDraw)
        {
            locationInfo.setText(locationString(center,start));
            zoomInfo.setText(std::to_string(zoom));
            drawMandelbrot(texture,center,start,zoom,julia);
        }
        if(update >= windowUPdateState::update)
        {
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            juliaTogle.draw();
            locationInfo.draw();
            zoomInfo.draw();
            reset.draw();
            SDL_RenderPresent(renderer);
        }

        update = windowUPdateState::noUpdate;
        mouseMove = {0,0};
        SDL_Delay(1000/30);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
