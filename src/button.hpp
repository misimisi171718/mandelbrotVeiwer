#pragma once

#include <array>
#include <string>
#include <functional>

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_events.h>

class button
{
private:
    SDL_Renderer* rend;
    SDL_Texture* texture;
    std::string text;
    std::function<void()> callback;
    SDL_Color color;
    SDL_Rect textRect;
    SDL_Rect buttonRect;
public:
    button(SDL_Renderer* rend, std::string text, SDL_Color col = {255,255,255,255});
    void onClick(std::function<void()> callback);
    void draw();
    void setText(std::string text);
    void setPos(SDL_Point pos);
    void mouseEvent(SDL_MouseButtonEvent& event);
};