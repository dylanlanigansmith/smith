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
    bool fullscreen;
    plat_graphics_params() : msaa_buffer(1), msaa_samples(2), tex_scale_quality("2"), fullscreen(false)
    {}
};


class CSystemWindow
{
friend class CEngine;
friend class CPlatform;
public:
    CSystemWindow() : m_setup(false) {}


    auto& GfxParams() const { return gfx_params; }
    auto Width() const { return width; }
    auto Height() const { return height; }
protected:
    bool SetupForPlatform(Platform::Platform_Types p){
        GatherDisplayInfo();
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

    bool CreateWindow(const char* title){
        m_SDLWindow = SDL_CreateWindow(title, width, height, m_windowFlags);
        if(gfx_params.fullscreen && m_SDLWindow != NULL)
            SDL_SetWindowFullscreen(m_SDLWindow, SDL_TRUE);


        return (m_SDLWindow != NULL);
    }
    void SetWindowSize(int w, int h){
        width = w; height = h;
    }

    void MakeFullScreen(){
        gfx_params.fullscreen = true;
    }
    void GatherDisplayInfo()
    {
        auto displays = SDL_GetDisplays(&m_numdisplays);
        if(displays == NULL){
            gError("failed to get display list [ size? %d], %s", m_numdisplays, SDL_GetError()); return;
        }
        int highest_res = 0;
        SDL_DisplayID best = 0;
        for(int i = 0; i < m_numdisplays; ++i)
        {
            auto display_mode = SDL_GetCurrentDisplayMode(displays[i]);
            if(display_mode == nullptr ){
                gError("failed to get display info for %d, %s", i, SDL_GetError());
            }
            if(display_mode->w > highest_res){
                highest_res = display_mode->w;
                best = displays[i];
            }
            gLog("display #%d : %dx%d @ %.1fhz [%d/%d]", displays[i], display_mode->w, display_mode->h, display_mode->refresh_rate, i, m_numdisplays);

        }
        gLog("using display %d as main", best);
        auto best_mode = SDL_GetCurrentDisplayMode(best);
        if(best_mode == NULL){
            gError("best display is null now so uh something went horribly wrong [%d]", best);
        }
        else
            m_displayMode = *best_mode;


        SDL_free(displays);
    }



private:
    int m_windowFlags;
    int width;
    int height;
    bool m_setup;
    plat_graphics_params gfx_params;
    SDL_Window* m_SDLWindow;
    int m_numdisplays;
    SDL_DisplayMode m_displayMode;
};