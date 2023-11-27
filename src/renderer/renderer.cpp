#include "renderer.hpp"

CRenderer::~CRenderer()
{
    if (m_renderer != nullptr)
        SDL_DestroyRenderer(m_renderer);
    log("Destroyed Renderer");
}

bool CRenderer::Create()
{
#ifdef __linux__
    return CreateRendererLinuxGL();
#endif

    return false;
}

bool CRenderer::CreateRendererLinuxGL()
{
    int numRenderers = SDL_GetNumRenderDrivers();
    log("found %i SDL renderers", numRenderers);
    for (int i = 0; i < numRenderers; ++i)
    {
        log("%i - %s", i, SDL_GetRenderDriver(i));
    }
    m_renderer = SDL_CreateRenderer(m_SDLWindow, NULL, SDL_RENDERER_ACCELERATED);
    if (0 > SDL_GetRendererInfo(m_renderer, &m_RendererInfo))
    {
        log("could not get post-init render info");
        return false;
    }
    log("created renderer-%s | max-texture {%ix%i}", m_RendererInfo.name, m_RendererInfo.max_texture_width, m_RendererInfo.max_texture_height);

    m_gl = SDL_GL_GetCurrentContext();
    if (m_gl == nullptr)
    {
        log("failed to acquire gl context from SDL");
        return false;
    }
    return (m_renderer != nullptr);
}

void CRenderer::Loop()
{
    SDL_Vertex hello[3] = {
        {.position = {100.f, 600.f}, .color = {255, 255, 0, 255}},
        {.position = {600.f, 600.f}, .color = {255, 255, 0, 255}},
        {.position = {100.f, 100.f}, .color = {255, 0, 0, 255}},
    };
    int indices[3] = {1, 2, 3};
    SDL_RenderClear(get());
    SDL_RenderGeometry(get(), NULL, hello, 3, NULL, 3);

    SDL_RenderPresent(get());
}
