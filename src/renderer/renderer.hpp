#pragma once

#include <common.hpp>
#include <global.hpp>
#include <types/Color.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <entity/player/CCamera.hpp>
// #include <entity/player/CPlayer.hpp>
#include "render_helpers.hpp"
#define SMITH_PIXELFMT SDL_PIXELFORMAT_RGBA8888

class CPlayer;

class CRenderer : public CLogger
{
public:
    CRenderer(SDL_Window *win) : CLogger("Render"), m_SDLWindow(win) {}
    virtual ~CRenderer();
    virtual bool Create();
    [[nodiscard]] auto get() const { return m_renderer; }

    virtual void Loop();
    virtual void Shutdown();
    const auto GetActiveCamera() { return m_Camera; }

    double ZBufferAt(int w)
    { /*assert(w < SCREEN_WIDTH);*/
        return ZBuffer[w];
    }
    void SetPixel(int x, int y, SDL_Color color);

    inline void SetPixel(int x, int y, const Color color)
    {
        int index = (y * m_surface->pitch / 4) + x;
        pixels[index] = color;
    }

    inline Color GetPixel(int x, int y)
    {
        int index = (y * m_surface->pitch / 4) + x;
        return Color(pixels[index]);
    }
    void OnEngineInitFinish();
private:
    bool CreateRendererLinuxGL();
    void RunImGui();
    void LoopWolf(int minX, int maxX);

    void inline DrawFloorCeiling(CPlayer *player, const int textW, const int textH, const int w, const int h,int minX, int maxX);

    void RenderSprites(CPlayer *player);

    void SetLightingRenderInfo();
    void UpdateLighting();

private:
    bool m_bThreadDone;
    double ZBuffer[SCREEN_WIDTH];
    uint32_t *pixels;
    SDL_Renderer *m_renderer;
    SDL_RendererInfo m_RendererInfo;
    SDL_Window *m_SDLWindow;
    SDL_GLContext m_gl;

    SDL_Texture *m_renderTexture;
    SDL_Texture *m_lightTexture;
    SDL_Surface *m_surface;
    SDL_Surface *m_lightsurface;
    CCamera *m_Camera;
};
