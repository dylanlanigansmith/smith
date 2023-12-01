#pragma once

#include <common.hpp>
#include <SDL3/SDL.h>

namespace Render
{
    static SDL_Color TextureToSDLColor(uint32_t abgr)
    {
        SDL_Color color;
        color.r =  ((abgr >> 24) & 0xFF);
        color.g =  ((abgr >> 16) & 0xFF);
        color.b =  ((abgr >> 8) & 0xFF);
        color.a =  (abgr & 0xFF);
        return color;
    }
    static uint32_t TextureToWorldColor(uint32_t abgr) //out rgba8888
    {
        SDL_Color color = TextureToSDLColor(abgr);
        uint32_t ret = (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
        return ret;
    }
    static uint32_t SDLColorToWorldColor(const SDL_Color& color)
    {
        //SDL_MapRGBA(surf->format, color.r, color.g, color.b, color.a);
         uint32_t ret = (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
        return ret;
    }
    static void DarkenSDLColor(SDL_Color& color, float val)
    {
        color.r = color.r / val;
        color.g = color.g / val;
        color.b = color.b / val;
    }

    static void SetPixel(uint32_t* pixels, int x, int y, int pitch, SDL_Color color){
        int index = (y * pitch / 4) + x;
        pixels[index] = SDLColorToWorldColor(color);
    }

    static inline bool ColorEqualRGB(const SDL_Color& c1, const SDL_Color& c2){
        return (c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b);
    }
}