#pragma once

#include <common.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>
#include <data/level.hpp>
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



    static inline bool ColorEqualRGB(const SDL_Color& c1, const SDL_Color& c2){
        return (c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b);
    }

    static SDL_Color MergeColors(SDL_Color f, SDL_Color b)
    {
        const float alpha = (f.a / 255.f);
        const float oalpha = 1.f - alpha;
        return {
            uint8_t(alpha * f.r + b.r * oalpha),
            uint8_t(alpha * f.g + b.g * oalpha),
            uint8_t(alpha * f.b + b.b * oalpha),
            255
        };
    }

    static inline uint32_t MergeColorsFast(const SDL_Color& f, const SDL_Color& b)
    {
        const float alpha = (f.a / 255.f);
        const float oalpha = 1.f - alpha;

         uint32_t ret = (uint8_t(alpha * f.r + b.r * oalpha) << 24) | (uint8_t(alpha * f.g + b.g * oalpha) << 16) | (uint8_t(alpha * f.b + b.b * oalpha) << 8) | 255;
        return ret;     
    }
    static inline uint32_t MergeColorsLazy(const SDL_Color& f, const SDL_Color& b) {
        const uint32_t alpha = f.a;  // Assuming f.a is already an integer in the range 0-255
        const uint32_t oalpha = 255 - alpha;

        uint32_t ret = ((alpha * f.r + b.r * oalpha) / 255) << 24 |
                    ((alpha * f.g + b.g * oalpha) / 255) << 16 |
                    ((alpha * f.b + b.b * oalpha) / 255) << 8 |
                    255;
        return ret;     
    }
    static inline uint32_t MergeColorsFixed(const SDL_Color& f, const SDL_Color& b, const float alpha, const float oalpha)
    {
         uint32_t ret = (uint8_t(alpha * f.r + b.r * oalpha) << 24) | (uint8_t(alpha * f.g + b.g * oalpha) << 16) | (uint8_t(alpha * f.b + b.b * oalpha) << 8) | 255;
        return ret;     
    }
    template <typename T>
    static bool IsInBounds(const T& value, const T& low, const T& high) {
            return !(value < low) && (value < high);
    }


   


    static inline Line_t GetLineForWallType(const IVector2& p, int type, int* side = nullptr){

        int pside;
         Line_t wall = {{0, 0}, {0, 0}};
        switch (type)
        {

        case Level::Tile_WallN:
          wall = {{(float)p.x, (float)p.y}, {(float)p.x + 1.0, (float)p.y}}; pside = 0;
          break;
        case Level::Tile_Door:
        case Level::Tile_WallE:
          wall = {{(float)p.x + 1.0, (float)p.y}, {(float)p.x + 1.0, (float)p.y + 1.0}}; pside = 1;
          break;
        case Level::Tile_WallS:
          wall = {{(float)p.x, (float)p.y + 1.0}, {(float)p.x + 1.0,(float) p.y + 1.0}}; pside = 0;
          break;
        case Level::Tile_WallW:
          wall = {{(float)p.x, (float)p.y}, {(float)p.x, p.y + 1.0}}; pside = 1;
          break;
        default:
          return wall;
        };
        if(side != nullptr)
            *side = pside;
        
        return wall;


    }
   
}