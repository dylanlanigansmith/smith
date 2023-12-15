#pragma once
#include <common.hpp>
#include <global.hpp>
#include <SDL3/SDL.h>
#include "platform_util.hpp"
#include <logger/logger.hpp>
#include <magic_enum/magic_enum.hpp>
struct plat_graphics_params
{
    int msaa_buffer, msaa_samples;
    std::string tex_scale_quality;

    plat_graphics_params() : msaa_buffer(1), msaa_samples(2), tex_scale_quality("2") 
    {}
};


class CSystemWindow
{
friend class CEngine;
public:
    CSystemWindow() : m_setup(false) {}


    auto& GfxParams() const { return gfx_params; }
protected:
    bool SetupForPlatform(Platform::Platform_Types p){
        if(p != Platform::WIN){
            m_setup = true;
            if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, MSAA_BUFFER) != 0){
                gError("failed to set GLAttr '%s',  Msg: %s ", 
                        magic_enum::enum_name(SDL_GL_MULTISAMPLEBUFFERS).data(), SDL_GetError());
                return false; 
            }
            if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, MSAA_SAMPLES) != 0){
                 gError("failed to set GLAttr '%s',  Msg: %s ", 
                        magic_enum::enum_name(SDL_GL_MULTISAMPLESAMPLES).data(), SDL_GetError());
                return false; 
            }
            if(SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, gfx_params.tex_scale_quality.c_str(), SDL_HINT_OVERRIDE) == SDL_FALSE){
                gError("failed to set Hit '%s' to '%s'", SDL_HINT_RENDER_SCALE_QUALITY, gfx_params.tex_scale_quality.c_str()); 
                //not critical  
            }
            m_windowFlags = SDL_WINDOW_OPENGL; //enough for now
            return true;
        }

        return false;
    }

    bool CreateWindow(const char* title, int w, int h){
        width = w; height = h;
        m_SDLWindow = SDL_CreateWindow(title, w, h, m_windowFlags);
         #ifdef SCREEN_FULLSCREEN
        if(m_SDLWindow != NULL)
            SDL_SetWindowFullscreen(m_SDLWindow, SDL_TRUE);
        #endif

        return (m_SDLWindow != NULL);
    }


private:
    int m_windowFlags;
    int width;
    int height;
    bool m_setup;
    plat_graphics_params gfx_params;
    SDL_Window* m_SDLWindow;
};