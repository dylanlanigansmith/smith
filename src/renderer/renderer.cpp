#include "renderer.hpp"

#include <types/Vector.hpp>
#include <thread>
#include <chrono>
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
   SDL_DestroyTexture(m_lightTexture);

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

  m_lightTexture = SDL_CreateTexture(get(), SMITH_PIXELFMT, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  SDL_SetTextureScaleMode(m_renderTexture, SDL_SCALEMODE_BEST);
  return ret;
}

void CRenderer::OnEngineInitFinish()
{
  static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
  SetLightingRenderInfo();
  ILightingSystem->SetupBlending();

  ILightingSystem->CalculateLighting();
}

void CRenderer::SetLightingRenderInfo()
{
  static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
  ILightingSystem->m_lightsurface = m_lightsurface;
  ILightingSystem->m_lighttexture = m_lightTexture;
  
}

void CRenderer::UpdateLighting()
{
  static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
  ILightingSystem->m_lightsurface = m_lightsurface;
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

  const SDL_FRect scale = {0.f,0.f, SCREEN_WIDTH_FULL, SCREEN_HEIGHT_FULL};

  SDL_LockTextureToSurface(m_renderTexture, NULL, &m_surface);
   SDL_LockTextureToSurface(m_lightTexture, NULL, &m_lightsurface); //https://wiki.libsdl.org/SDL3/SDL_LockTextureToSurface
   SetLightingRenderInfo();
  if(SDL_MUSTLOCK(m_surface)) //seems to not need to
    SDL_LockSurface(m_surface);

  pixels = (uint32_t *)m_surface->pixels;


 // std::thread half(&CRenderer::LoopWolf,this,0, SCREEN_WIDTH / 2);
  m_bThreadDone = false;
 
  LoopWolf(0, SCREEN_WIDTH);

  //while(!m_bThreadDone && true){
  //  std::this_thread::sleep_for(std::chrono::microseconds(15));
 // }
  //if(half.joinable())
  //  half.join();
  
  

  if(m_surface->locked == SDL_TRUE)
    SDL_UnlockSurface(m_surface);
  SDL_UnlockTexture(m_renderTexture);

  SDL_UnlockTexture(m_lightTexture);
 
  if(SCREEN_HEIGHT == SCREEN_HEIGHT_FULL) SDL_RenderTexture(get(), m_renderTexture, NULL, NULL);  
  else SDL_RenderTexture(get(), m_renderTexture, NULL, &scale);
  
  if(SCREEN_HEIGHT == SCREEN_HEIGHT_FULL) SDL_RenderTexture(get(), m_lightTexture, NULL, NULL);  
  else SDL_RenderTexture(get(), m_lightTexture, NULL, &scale);
 
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


