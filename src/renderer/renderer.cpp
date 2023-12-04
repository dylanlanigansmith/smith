#include "renderer.hpp"

#include <types/Vector.hpp>

#include <engine/engine.hpp>

#include <GL/gl.h>
#include <interfaces/interfaces.hpp>
#include <interfaces/IEngineTime/IEngineTime.hpp>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_impl_sdl3.h>
#include <editor/editor.hpp>

void CRenderer::Shutdown()
{
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyTexture(m_renderTexture);
  

  SDL_DestroyRenderer(m_renderer);
  log("Destroyed Renderer");
}



CRenderer::~CRenderer()
{
}

bool CRenderer::Create()
{
  bool ret = false;

#ifdef __linux__
  ret = CreateRendererLinuxGL();
#endif

  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForSDLRenderer(m_SDLWindow, get());
  ImGui_ImplSDLRenderer3_Init(get());

  m_renderTexture = SDL_CreateTexture(get(), SMITH_PIXELFMT, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

  
  return ret;
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


#define mapWidth 24
#define mapHeight 24


void CRenderer::Loop()
{

  SDL_FRect scale = {0.f,0.f, SCREEN_WIDTH_FULL, SCREEN_HEIGHT_FULL};

  SDL_LockTextureToSurface(m_renderTexture, NULL, &m_surface);
  LoopWolf();
  SDL_UnlockTexture(m_renderTexture);
 //https://wiki.libsdl.org/SDL3/SDL_LockTextureToSurface
  if(SCREEN_HEIGHT == SCREEN_HEIGHT_FULL) SDL_RenderTexture(get(), m_renderTexture, NULL, NULL);
    
  else SDL_RenderTexture(get(), m_renderTexture, NULL, &scale);
    
 
  RunImGui();
 
  SDL_RenderPresent(get());
}


void CRenderer::RunImGui()
{
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

   //ImGui::ShowDemoWindow();
  CEditor::instance().render(this);
  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());

}

void CRenderer::SetPixel(int x, int y, SDL_Color color) // @deprecated Switch To Color class
{
  #warning "Using SDLColor is deprecated"
  int index = (y * m_surface->pitch / 4) + x;
  pixels[index] = Render::SDLColorToWorldColor(color);
}
/*
SDL_Color CRenderer::GetPixel(int x, int y)
{
   int index = (y * m_surface->pitch / 4) + x;
  return Render::TextureToSDLColor( pixels[index]);
}*/


