#pragma once

#include <SDL2/SDL_image.h>
#include <complex>

void drawMandelbrot(
        SDL_Texture* text,
        const std::complex<double>& center,
        const std::complex<double>& start,
        const double& zoom,
        const bool julia);
void openclInit();