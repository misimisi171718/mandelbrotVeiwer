#include <iostream>
#include <fstream>
#include <chrono>
#include <math.h>
#include <SDL2/SDL.h>
#include <complex>
#include <array>

#include "opencl.hpp"

#define WINDOW_WIDTH 700

enum windowUPdateState{
    noUpdate = 0,
    update = 1,
    reDraw = 2,
    reSize = 3
};

using namespace std::complex_literals;

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
                break;
            }
            
            }
        }
        auto mouseState = SDL_GetMouseState(nullptr,nullptr);
        if(mouseState & SDL_BUTTON_LMASK)
        {
            int winH,winW;
            SDL_GetWindowSize(window,&winW,&winH);
            center -= mouseMove[1]/std::abs(zoom*winH) * 1.0i;
            center -= mouseMove[0]/std::abs(zoom*winW);
            update = windowUPdateState::reDraw;
        }
        if(mouseState & SDL_BUTTON_RMASK)
        {
            int winH,winW;
            SDL_GetWindowSize(window,&winW,&winH);
            start -= mouseMove[1]/std::abs(zoom*winH) * 1.0i;
            start -= mouseMove[0]/std::abs(zoom*winW);
            update = windowUPdateState::reDraw;
        }

        if(update >= windowUPdateState::reSize)
            resizeWindow(renderer,texture,window);
        if(update >= windowUPdateState::reDraw)
            drawMandelbrot(texture,center,start,zoom,julia);
        if(update >= windowUPdateState::update)
            updateWindow(renderer,texture);

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
