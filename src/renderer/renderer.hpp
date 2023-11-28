#pragma once

#include <common.hpp>
#include <global.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>

#define SMITH_PIXELFMT SDL_PIXELFORMAT_RGBA8888

class CRenderer : public CLogger
{
    public:
        CRenderer(SDL_Window* win) : CLogger("Render"), m_SDLWindow(win) { }
        virtual ~CRenderer();
        virtual bool Create();
        [[nodiscard]]  auto get() const { return m_renderer; }

        virtual void Loop();
        virtual void Shutdown();
    private:
        bool CreateRendererLinuxGL();
        void RunImGui();
    private:
        double ZBuffer[SCREEN_WIDTH];
        SDL_Renderer* m_renderer;
        SDL_RendererInfo m_RendererInfo;
        SDL_Window* m_SDLWindow;
        SDL_GLContext m_gl;

        SDL_Texture* m_renderTexture;
        SDL_Surface* m_surface;
};
