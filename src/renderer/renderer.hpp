#pragma once

#include <common.hpp>

#include <SDL3/SDL.h>
#include <logger/logger.hpp>



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

    private:
        SDL_Renderer* m_renderer;
        SDL_RendererInfo m_RendererInfo;
        SDL_Window* m_SDLWindow;
        SDL_GLContext m_gl;
};
