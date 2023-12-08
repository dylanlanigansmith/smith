#pragma once
#include <common.hpp>
#include <types/Vector.hpp>
#include <types/Color.hpp>
#include <imgui.h>
#include <SDL3/SDL.h>
#include <magic_enum/magic_enum.hpp>
namespace Editor
{
    inline std::string IVec2Str(const IVector2& v){
        return std::string(std::to_string(v.x) + std::string(" ") + std::to_string(v.y)); //this is what i mean by a little greasy
    }

    inline ImVec4 SDLClrToImClr4(const SDL_Color& clr)
    {
        return ImVec4(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f, 1.f);
    }
    inline SDL_Color AvgColor(const SDL_Color& c1, const SDL_Color& c2, const SDL_Color& c3, const SDL_Color& c4){
        int r = c1.r + c2.r + c3.r + c4.r, g = c1.g + c2.g + c3.g + c4.g, b = c1.b + c2.b + c3.b + c4.b;
        return SDL_Color{
            .r = (uint8_t)(r / 4),
            .g = (uint8_t)(g / 4),
            .b = (uint8_t)(b / 4),
            .a = 255
        };
    }
    inline SDL_Color GenerateAverageColor(texture_t* texture){
         int samplePt = texture->m_size.x / 2;
        SDL_Color clr = Render::TextureToSDLColor(texture->getColorAtPoint(samplePt, samplePt));
        SDL_Color clr2 = Render::TextureToSDLColor(texture->getColorAtPoint(samplePt + 2, samplePt + 2));
        SDL_Color clr3 = Render::TextureToSDLColor(texture->getColorAtPoint(samplePt - 2, samplePt - 2)); //arbitrary 
        SDL_Color clr4 = Render::TextureToSDLColor(texture->getColorAtPoint(samplePt + 1, samplePt - 1));
        return Editor::AvgColor(clr, clr2, clr3, clr4);
    }
     template <typename T> 
     const std::string_view GetEnumName(T en)
     {
        return magic_enum::enum_name(en);
     }
   

   inline void ColorPreview(Color c, const ImVec2 size = ImVec2(32,32) ){
        ImGui::PushID(&(c) );
        ImVec4 imclr = ImGui::ColorConvertU32ToFloat4(IM_COL32(c.r(),c.g(),c.b(),c.a() ));
        ImGui::ColorButton("##mask", imclr, ImGuiColorEditFlags_NoPicker, {32,32});
        ImGui::PopID();
   }
    inline Color Im4ToColor(const ImVec4& v){
        return Color(
            v.x, v.y, v.z, v.w, 255
        );
    }
   inline ImVec4 ColorToIm4(const Color& clr, bool alpha = false)
    {
        return ImVec4(clr.r() / 255.f, clr.g() / 255.f, clr.b() / 255.f, (alpha) ? clr.a() /255.f : 1.f);
    }
    inline ImU32 ColorToIU32(const Color& c, bool alpha = false)
    {
        return IM_COL32(c.r(),c.g(),c.b(), (alpha) ? c.a() : 255);
    }

    inline Color colorPicker(const char* label, Color in, int flags){
        auto c = ColorToIm4(in, true);

        ImGui::ColorEdit4(label, &c.x, flags);

        return Im4ToColor(c);
    }
}