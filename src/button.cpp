#include "button.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

static TTF_Font* getFont()
{
    static TTF_Font* font;
    if(font)
        return font;
    
    TTF_Init();
    font = TTF_OpenFont("JetBrainsMono-Light.ttf", 15 );
    return font;
}

void button::draw()
{
    SDL_SetRenderDrawColor(rend,color.r,color.g,color.b,color.a);
    SDL_RenderFillRect(rend,&buttonRect);
    SDL_RenderCopy(rend,texture,nullptr,&textRect);
}

void button::setText(std::string text) 
{
    this->text = text;
    if(texture)
        SDL_DestroyTexture(texture);
    auto tmpSurface = TTF_RenderUTF8_Blended(getFont(),text.c_str(),{0,0,0,255});
    texture = SDL_CreateTextureFromSurface(rend,tmpSurface);
    SDL_FreeSurface(tmpSurface);
    SDL_QueryTexture(texture,nullptr,nullptr,&textRect.w,&textRect.h);
    textRect.x = buttonRect.x + 5;
    textRect.y = buttonRect.y + 5;
    buttonRect.w = textRect.w + 10;
    buttonRect.h = textRect.h + 10;
}

void button::setPos(SDL_Point pos) 
{
    buttonRect.x = pos.x;
    buttonRect.y = pos.y;
    textRect.x = pos.x + 5;
    textRect.y = pos.y + 5;
}

void button::mouseEvent(SDL_MouseButtonEvent& event) 
{
    SDL_Point mouse;
    mouse.x = event.x;
    mouse.y = event.y;
    if(SDL_PointInRect(&mouse,&buttonRect))
        if(callback)
            callback();
    
}
button::button(SDL_Renderer* rend, std::string text, SDL_Color col)
:rend(rend),textRect(),buttonRect(),color(col),texture(nullptr)
{
    setText(text);
}

void button::onClick(std::function<void()> callback) 
{
    this->callback = callback;
}