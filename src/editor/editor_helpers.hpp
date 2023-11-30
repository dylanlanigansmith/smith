#pragma once
#include <common.hpp>
#include <types/Vector.hpp>
#include <imgui.h>
#include <SDL3/SDL.h>

namespace Editor
{
    inline std::string IVec2Str(const IVector2& v){
        return std::string(std::to_string(v.x) + std::string(" ") + std::to_string(v.y)); //this is what i mean by a little greasy
    }

    inline ImVec4 SDLClrToImClr4(const SDL_Color& clr)
    {
        return ImVec4(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f, 1.f);
    }
}