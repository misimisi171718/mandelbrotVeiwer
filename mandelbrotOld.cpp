#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <math.h>

#define WINDOW_WIDTH 700

struct complexNumber
{
    double real, imagenary;
    complexNumber& operator*(const complexNumber num)
    {
        double temp = real;
        real = (num.real * real) - (num.imagenary * imagenary);
        imagenary = (num.imagenary * temp) + (num.real * imagenary);
        return *this;
    }
    complexNumber& operator/(const complexNumber num)
    {
        double x = (num.imagenary*num.imagenary)+(num.real*num.real);
        real = ((num.real * real) + (num.imagenary * imagenary))/x;
        imagenary = ((num.imagenary * real) - (num.real * imagenary))/x;
        return *this;
    }
    complexNumber& operator+(const complexNumber num)
    {
        real = real + num.real;
        imagenary = imagenary + num.imagenary;
        return *this;
    }
    complexNumber& operator-(const complexNumber num)
    {
        real -= num.real;
        imagenary -= num.imagenary;
        return *this;
    }
};

std::ostream& operator<<(std::ostream& st, const complexNumber& num)
{
    st << num.real << " " << num.imagenary;
    return st;
}

struct color
{
    u_int8_t r, g, b;
};

color colorMap(int num)
{
    color col;
    col.r = (num % 4) * 64;
    col.g = ((num / 4) % 8) * 64;
    col.b = ((num / 16) % 8) * 64;
    return col;
}

int mandelbrotPixel(complexNumber c, size_t iterations)
{
    complexNumber z = {0,0};
    for (size_t i = 0; i < iterations; i++)
    {
        z = (z*z)+c;
        if(pow(z.real,2)+pow(z.imagenary,2) >= 6 )
            return i;
    }
    return -1;
}

void drawMandelbrot(
        SDL_Renderer* rend,
        SDL_Texture* text,
        const complexNumber& center,
        const double& zoom)
{
    auto start = std::chrono::high_resolution_clock::now();
    int winW, winH;
    SDL_QueryTexture(text, NULL, NULL, &winW, &winH);
    SDL_SetRenderTarget(rend, text);
    double Wdiv = winW * zoom;
    double Hdiv = winH * zoom * ((double)winW / (double)winH);
    double Wadd = 1 / (zoom * 2);
    double Hadd = 1 / (zoom * 2 * ((double)winW / (double)winH));
    size_t iteraions = log2(zoom*8)*40+100;
    for (size_t x = 0; x < winW; x++) 
    {
        for (size_t y = 0; y < winH; y++)
        {
            double cX = ((double)x / Wdiv) + center.real - Wadd;
            double cY = ((double)y / Hdiv) + center.imagenary - Hadd;
            int j = mandelbrotPixel({cX,cY},iteraions);
            if(j == -1)
                SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
            else
            {
                color col = colorMap(j);
                SDL_SetRenderDrawColor(rend, col.r, col.g, col.b, 0);
            }
            SDL_RenderDrawPoint(rend, x, y);
        }
    }
    SDL_SetRenderTarget(rend, NULL);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
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
    text = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
}

int main()
{
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *texture;
    int i;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, SDL_WINDOW_RESIZABLE, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_WIDTH);
    complexNumber center = { 0, 0};
    double zoom = 0.25;
    bool open = true;
    
    while (open) {
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                open = false;
            if (event.type == SDL_WINDOWEVENT)
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    resizeWindow(renderer, texture, window);
                    drawMandelbrot(renderer,texture,center,zoom);
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
            if (event.type == SDL_MOUSEBUTTONUP)
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int winW, winH;
                    SDL_QueryTexture(texture, NULL, NULL, &winW, &winH);
                    center.real      += ( (1 / zoom)                                / winW) * (event.button.x - (winW / 2));
                    center.imagenary += (((1 / zoom) * ((double)winW / (double)winH)) / winH) * (event.button.y - (winH / 2));
                    zoom = zoom * 2;
                    drawMandelbrot(renderer,texture,center,zoom);
                    updateWindow(renderer,texture);
                }
        }
        SDL_Delay(100);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}