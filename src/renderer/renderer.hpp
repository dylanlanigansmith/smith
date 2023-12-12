#pragma once

#include <common.hpp>
#include <global.hpp>
#include <types/Color.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include <entity/player/CCamera.hpp>
// #include <entity/player/CPlayer.hpp>
#include "render_helpers.hpp"


class CPlayer;

class CRenderer : public CLogger
{
public:
    CRenderer(SDL_Window *win) : CLogger(std::string("Render")), m_SDLWindow(win) {}
    virtual ~CRenderer();
    virtual bool Create();
    [[nodiscard]] auto get() const { return m_renderer; }

    virtual void Loop();
    virtual void Shutdown();
    const auto GetActiveCamera() { return m_Camera; }

    inline double ZBufferAt(int w)
    { /*assert(w < SCREEN_WIDTH);*/
        return ZBuffer[w];
    }
    inline void SetZBuffer(int idx, double val){
        ZBuffer[idx] = val;
    }
    void SetPixel(int x, int y, SDL_Color color);

    inline void SetPixel(int x, int y, const Color color)
    {
        int index = (y * m_surface->pitch / 4) + x;
        pixels[index] = color;
    }
    inline void SetPixel(SDL_Surface* surf, int x, int y, const Color color)
    {
        int index = (y * surf->pitch / 4) + x;
        ((uint32_t*)surf->pixels  )[index] = color;
    }
    inline Color GetPixel(SDL_Surface* surf,int x, int y)
    {
        int index = (y * surf->pitch / 4) + x;
        return Color(((uint32_t*)surf->pixels  )[index]);
    }
    inline Color GetPixel(int x, int y)
    {
        int index = (y * m_surface->pitch / 4) + x;
        return Color(pixels[index]);
    }
    void OnEngineInitFinish();

 
    
    uint8_t Z2D[SCREEN_WIDTH][SCREEN_HEIGHT] = {0}; //oh god
private:
    bool CreateRendererLinuxGL();
    void RunImGui();
    void LoopWolf(int minX, int maxX, bool sprites = true);

    void  DrawFloorCeiling(CPlayer *player, const int textW, const int textH, const int w, const int h,int minX, int maxX);

    void RenderSprites(CPlayer *player);

    void SetLightingRenderInfo();
    void UpdateLighting();
    void BlurTexture();
    void GaussianBlurPass(bool horizontal, int startX, int endX);
    void GaussBlurTexture(int startX, int endX);
    void GenerateGaussKernel();

    void SetupThreads();
private:
    int thread_count;
    std::atomic<bool> startRender{false};
    std::atomic<bool> startBlur{false};
    std::atomic<bool> stopThread{false};
    std::atomic<int> doneCount{0};
    
    std::vector<std::thread> workers;
    std::mutex mutex;
    std::mutex doneMutex;
    


    std::vector<float> kernel;
    bool m_bBlur = false;
    bool m_bBlurGauss = true;
    int kernelSize;
     float sigma;
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

    SDL_Texture *m_blurTexture;
    SDL_Surface *m_blur;
    SDL_Surface *m_downscale;
    CCamera *m_Camera;
};
