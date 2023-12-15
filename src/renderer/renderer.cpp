#pragma GCC optimize("O2")
#include "renderer.hpp"
#include "render2/render2.hpp"
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
  startRender.store(false);
  startBlur.store(false);
  stopThread.store(true);

  for (auto &t : workers)
  {
    t.join();
  }
  log("killed threads");
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyTexture(m_renderTexture);
  SDL_DestroyTexture(m_lightTexture);
  SDL_DestroySurface(m_downscale);
  SDL_DestroySurface(m_lightsurface);
  SDL_DestroyRenderer(m_renderer);
  log("Destroyed Renderer");
}

CRenderer::~CRenderer()
{
}

bool CRenderer::Create()
{
  auto plat_type = PLATFORM.GetPlatType();
  windowSize.x = PLATFORM.SysWindow().Width();
  windowSize.y = PLATFORM.SysWindow().Height();
  log("%d %d ", windowSize.x, windowSize.y);
  m_isUpScaling = (SCREEN_HEIGHT != windowSize.y);


  m_bBlur = true;
  
  m_bBlurMethod = false;
  sigma = 8.3f;   // higher = softer
  kernelSize = 5; // higher = more area
  avg_kernelSize = 4;
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

  // m_lightTexture = SDL_CreateTexture(get(), SMITH_PIXELFMT, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
  m_lightsurface = SDL_CreateSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SMITH_PIXELFMT);
  m_downscale = SDL_CreateSurface(SCREEN_WIDTH / BLUR_SCALE, SCREEN_HEIGHT / BLUR_SCALE, SMITH_PIXELFMT);

  m_blurTexture = SDL_CreateTexture(get(), SMITH_PIXELFMT, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH / BLUR_SCALE, SCREEN_HEIGHT / BLUR_SCALE);

  SDL_SetTextureScaleMode(m_renderTexture, SDL_SCALEMODE_BEST);
  // SDL_SetTextureScaleMode(m_blurTexture, SDL_SCALEMODE_BEST);

  thread_count = engine->GetSysInfo().render_threads_to_use;

  SetupThreads();
  note("using %d threads for {%d x %d} blur: (%0.4f %d)", thread_count, SCREEN_WIDTH, SCREEN_HEIGHT, sigma, kernelSize);
  return ret;
}

void CRenderer::OnEngineInitFinish()
{
  static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
  SetLightingRenderInfo();
  ILightingSystem->SetupBlending();

  ILightingSystem->CalculateLighting();
  SDL_SetTextureBlendMode(m_blurTexture, SDL_BLENDMODE_BLEND);
  SDL_SetSurfaceBlendMode(m_downscale, SDL_BLENDMODE_NONE);
  SDL_SetSurfaceBlendMode(m_lightsurface, SDL_BLENDMODE_NONE);

  GenerateGaussKernel();
}

void CRenderer::SetLightingRenderInfo()
{
  static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");

  ILightingSystem->m_lightsurface = (m_bBlur) ? m_lightsurface : m_blur;
  // ILightingSystem->m_lighttexture = m_lightTexture;
}

void CRenderer::UpdateLighting()
{
  static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
}
void CRenderer::applyMovingAverage(int startX, int endX, int startY, int endY)
{

  static SDL_Surface *surf = (BLUR_SCALE != 1) ? m_downscale : m_lightsurface;
  static constexpr int width = SCREEN_WIDTH / BLUR_SCALE;
  static constexpr int height = SCREEN_HEIGHT / BLUR_SCALE;
  static const int pitch = surf->pitch / 4;
  const int ksz = avg_kernelSize / 2;
  const int leftMargin = ksz;
  const int rightMargin = width - ksz;
  const int topMargin = ksz;
  const int bottomMargin = height - ksz;

  struct pixel_clr {
    uint8_t a,b,g,r;
  };
//same code 3 times to semi unroll loop so that clamps only done when needed
  if (startX == 0 || endX == width ) //all bounds checks needed
  {
    for (int y = startY; y < endY; ++y)
    {
      for (int x = startX; x < endX; ++x)
      {
        if (x < leftMargin || x >= rightMargin || y < topMargin || y >= bottomMargin)
        {
          uint32_t r = 0, g = 0, b = 0, a = 0;
          int count = 0;

          // Kernel iteration
          for (int ky = -ksz; ky <= ksz; ++ky)
          {
            for (int kx = -ksz; kx <= ksz; ++kx)
            {
              int sampleX = std::clamp(x + kx, 0, width - 1);
              int sampleY = std::clamp(y + ky, 0, height - 1);
               pixel_clr c = ((pixel_clr *)surf->pixels)[(sampleY * pitch) + sampleX];

              r += c.r;
              g += c.g;
              b += c.b;
              a += c.a;
             
              count++;
            }
          }

          // Compute average
          r /= count;
          g /= count;
          b /= count;
          a /= count;

           ((pixel_clr *)(m_blur->pixels))[(y * pitch) + x] =  { a,b,g,r  };
        }
      }
    }
  }
  else //no X checks needed
  {
    for (int y = startY; y < endY; ++y)
    {
      for (int x = startX; x < endX; ++x)
      {
        if (x < leftMargin || x >= rightMargin || y < topMargin || y >= bottomMargin)
        {
          uint32_t r = 0, g = 0, b = 0, a = 0;
          int count = 0;

          // Kernel iteration
          for (int ky = -ksz; ky <= ksz; ++ky)
          {
            for (int kx = -ksz; kx <= ksz; ++kx)
            {
              int sampleX = x + kx;
              int sampleY = std::clamp(y + ky, 0, height - 1);
                pixel_clr c = ((pixel_clr *)surf->pixels)[(sampleY * pitch) + sampleX];

              r += c.r;
              g += c.g;
              b += c.b;
              a += c.a;
             
              count++;
            }
          }

          // Compute average
          r /= count;
          g /= count;
          b /= count;
          a /= count;

           ((pixel_clr *)(m_blur->pixels))[(y * pitch) + x] =  { a,b,g,r  };
        }
      }
    }
  }
//int y = startY; y < endY;
  const int x_start = std::max(startX, leftMargin), x_end = std::min(endX, rightMargin);
  // Process central area without boundary checks
  for (int y = topMargin; y < bottomMargin; ++y)
  {
    for (int x = x_start; x < x_end; ++x)
    {
      uint32_t r = 0, g = 0, b = 0, a = 0;
      int count = 0;

      // Kernel iteration
      for (int ky = -ksz; ky <= ksz; ++ky)
      {
        for (int kx = -ksz; kx <= ksz; ++kx)
        {
          int sampleX = x + kx;
          int sampleY = y + ky;

          pixel_clr c = ((pixel_clr *)surf->pixels)[(sampleY * pitch) + sampleX];

          r += c.r;
          g += c.g;
          b += c.b;
          a += c.a;
          count++;
        }
      }

      // Compute average
      r /= count;
      g /= count;
      b /= count;
      a /= count;

      ((pixel_clr *)(m_blur->pixels))[(y * pitch) + x] =  { a,b,g,r  };// ((r) << 24) | ((g) << 16) | ((b) << 8) | ((a));
    }
  }

  //Render::Blur(): Avg. (325008 ns) (325 us) (0 ms) (0.000325 sec) Wed Dec 13 18:02:54 2023
  
  //Render::Blur(): Avg. (336435 ns) (336 us) (0 ms) (0.000336 sec) Wed Dec 13 18:07:48 2023

  //Render::Blur(): Avg: { (332 us) (0 ms) }  Max:  (4942 us)  | Min: (319 us) @ Wed Dec 13 18:14:18 2023



}
void CRenderer::applyRollingAverage(int startY, int endY, int kernelSize)
{
  struct big_color_t
  {
    uint64_t r{}, g{}, b{}, a{};

    void operator+=(const Color &rhs)
    {
      r += rhs.r();
      g += rhs.g();
      b += rhs.b();
      a += rhs.a();
    }
    void operator+=(const big_color_t &rhs)
    {
      r += rhs.r;
      g += rhs.g;
      b += rhs.b;
      a += rhs.a;
    }
    void add(const Color &color)
    {
      r += color.r();
      g += color.g();
      b += color.b();
      a += color.a();
    }

    void subtract(const big_color_t &other)
    {
      r -= other.r;
      g -= other.g;
      b -= other.b;
      a -= other.a;
    }
    Color operator/(const int c) const
    {
      return Color(uint8_t(r / c), uint8_t(g / c), uint8_t(b / c), uint8_t(a / c));
    }
    Color average(int area) const
    {
      return Color(r / area, g / area, b / area, a / area);
    }
  };

  SDL_Surface *surf = (BLUR_SCALE != 1) ? m_downscale : m_lightsurface;
  int width = SCREEN_WIDTH / BLUR_SCALE;
  int height = SCREEN_HEIGHT / BLUR_SCALE;
  int extendedStartY = std::max(startY - kernelSize / 2, 0);
  int extendedEndY = std::min(endY + kernelSize, height);
  std::vector<big_color_t> horizontalSum(width * height);

  // Horizontal pass
  for (int y = startY; y < extendedEndY; ++y)
  {
    big_color_t x_color;
    for (int x = 0; x < width; ++x)
    {
      Color clr = GetPixel(surf, x, y);
      x_color += clr;
      //  log( "r%li g%li b%li a%li thread=%d", x_color.r, x_color.g, x_color.b, x_color.a, startX);
      horizontalSum[y * width + x] = x_color;
    }
  }
  std::vector<big_color_t> verticalCumulativeSum(height * width);
  int kernelArea = kernelSize * kernelSize;

  for (int x = 0; x < width; ++x)
  {
    big_color_t sum = {};
    for (int y = 0; y < startY; ++y)
    {
      sum += horizontalSum[y * width + x];
    }
    for (int y = startY; y < extendedEndY; ++y)
    {
      sum += horizontalSum[y * width + x];
      verticalCumulativeSum[y * width + x] = sum;

      if (x >= kernelSize - 1 && y >= kernelSize - 1)
      {
        big_color_t topLeft = (x - kernelSize < 0 || y - kernelSize < 0) ? big_color_t{} : verticalCumulativeSum[(y - kernelSize) * width + (x - kernelSize)];
        big_color_t topRight = (y - kernelSize < 0) ? big_color_t{} : verticalCumulativeSum[(y - kernelSize) * width + x];
        big_color_t bottomLeft = (x - kernelSize < 0) ? big_color_t{} : verticalCumulativeSum[y * width + (x - kernelSize)];

        big_color_t areaSum = sum;
        areaSum.subtract(topRight);
        areaSum.subtract(bottomLeft);
        areaSum += topLeft;

        SetPixel(m_blur, x, y, areaSum.average(kernelArea));
      }
    }
  }
}

void CRenderer::BlurTexture()
{

  int blurRadius = 1; // You can adjust this for a more or less blurred effect
  SDL_Surface *surf = m_lightsurface;
  int width = SCREEN_WIDTH / BLUR_SCALE;
  int height = SCREEN_HEIGHT / BLUR_SCALE;
  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      int r = 0, g = 0, b = 0, a = 0;
      int count = 0;

      // Sum up the color values of the neighboring pixels
      for (int dy = -blurRadius; dy <= blurRadius; ++dy)
      {
        for (int dx = -blurRadius; dx <= blurRadius; ++dx)
        {
          int nx = x + dx;
          int ny = y + dy;

          // Check bounds
          if (nx >= 0 && nx < width && ny >= 0 && ny < height)
          {
            Color clr = GetPixel(surf, nx, ny);
            r += clr.r();
            g += clr.g();
            b += clr.b();
            a += clr.a();
            count++;
          }
        }
      }

      // Calculate the average color
      r /= count;
      g /= count;
      b /= count;
      a /= count;
      // Assign the blurred pixel to the temporary buffer
      SetPixel(m_blur, x, y, Color(r, g, b, a));
    }
  }
}
void CRenderer::GaussianBlurPass(bool horizontal, int startX, int endX)
{
  SDL_Surface *surf = (BLUR_SCALE != 1) ? m_downscale : m_lightsurface;
  constexpr int width = SCREEN_WIDTH / BLUR_SCALE;
  constexpr int height = SCREEN_HEIGHT / BLUR_SCALE;
  const int pitch = surf->pitch / 4;
  for (int y = 0; y < height; ++y)
  {
    for (int x = startX; x < endX; ++x)
    {
      float r = 0, g = 0, b = 0, a = 0;

      for (int k = -kernelSize / 2; k <= kernelSize / 2; ++k)
      {
        int sampleX = horizontal ? std::clamp(x + k, 0, width - 1) : x;
        int sampleY = horizontal ? y : std::clamp(y + k, 0, height - 1);

        int sampleIndex = (sampleY * pitch) + sampleX; // Corrected index calculation
        Color clr = ((uint32_t *)surf->pixels)[sampleIndex];

        float weight = kernel[k + kernelSize / 2];
        r += clr.r() * weight;
        g += clr.g() * weight;
        b += clr.b() * weight;
        a += clr.a() * weight;
      }

      ((uint32_t *)(m_blur->pixels))[(y * pitch) + x] = Color(r, g, b, a); //
    }
  }
}

void CRenderer::GaussBlurTexture(int startX, int endX)
{

  GaussianBlurPass(true, startX, endX);  // Horizontal pass
  GaussianBlurPass(false, startX, endX); // Vertical pass
}
void CRenderer::GenerateGaussKernel()
{
  kernel.resize(kernelSize);
  // Generate Gaussian kernel
  float sum = 0.0f;
  for (int i = 0; i < kernelSize; ++i)
  {
    int x = i - kernelSize / 2;
    kernel[i] = exp(-(x * x) / (2 * sigma * sigma));
    sum += kernel[i];
  }
  for (float &value : kernel)
    value /= sum;
}

#define NUM_THREADS this->thread_count
void CRenderer::SetupThreads()
{

  for (int i = 0; i < NUM_THREADS; ++i)
  {

    workers.emplace_back([this, i]
                         {
                this->dbg("thread %d (%d -> % d)",i, 0 + ( i * (SCREEN_WIDTH / NUM_THREADS)), (SCREEN_WIDTH / NUM_THREADS) * (i + 1) );
                while (!stopThread.load()) {
                   while (!this->startRender.load() ) {
                        if(this->stopThread.load()) return;
                        
                        std::this_thread::yield(); // Avoid busy-waiting
                    }
                    
                    // make sure we make it to the end if it isnt an even number
                    const int end = (i == NUM_THREADS - 1) ? (SCREEN_WIDTH) :  (SCREEN_WIDTH / NUM_THREADS) * (i + 1);
                    this->LoopWolf(0 + ( i * (SCREEN_WIDTH / NUM_THREADS)), end, i == 0 );

                    doneCount.fetch_add(1);
                   
                     while (this->startRender.load() || !this->startBlur.load()) {
                        std::this_thread::yield();
                    }
                    const int endblur = (i == NUM_THREADS - 1) ? (SCREEN_WIDTH / BLUR_SCALE): (((SCREEN_WIDTH / BLUR_SCALE))/ NUM_THREADS) * (i + 1);
                    const int startblur = 0 + ( i * ((SCREEN_WIDTH / BLUR_SCALE)) / NUM_THREADS) ;

                    const int startblury = 0 + ( i * ((SCREEN_HEIGHT / BLUR_SCALE)) / NUM_THREADS);
                    const int endblury = (i == NUM_THREADS - 1) ? (SCREEN_HEIGHT / BLUR_SCALE): (((SCREEN_HEIGHT/ BLUR_SCALE))/ NUM_THREADS) * (i + 1);
                    if(this->m_bBlurMethod)
                      this->GaussBlurTexture( startblur ,  endblur ); 
                    else
                      this->applyMovingAverage(startblur,  endblur); 
                  // 
                   // this->applyRollingAverage( startblury  ,  endblury, 8 );
                     doneCount.fetch_add(1);
                    while(this->startBlur.load() ){
                      std::this_thread::yield();
                    }
                   

                } });
  }
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
  static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
  static auto WolfProfiler = IEngineTime->AddProfiler("Render::LoopWolf()");
  static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
  static auto BlurProfiler = IEngineTime->AddProfiler("Render::Blur()");
  static auto SpriteProfiler = IEngineTime->AddProfiler("Render::Sprites()");
  static auto SDLProfilerStart = IEngineTime->AddProfiler("Render::SDLRendererStart");
  static auto SDLProfiler = IEngineTime->AddProfiler("Render::SDLRendererEnd");
  static const SDL_FRect scale = {0.f, 0.f, windowSize.x, windowSize.y};

  SDLProfilerStart->Start();
  SDL_LockTextureToSurface(m_renderTexture, NULL, &m_surface);
  // SDL_LockTextureToSurface(m_lightTexture, NULL, &m_lightsurface); //https://wiki.libsdl.org/SDL3/SDL_LockTextureToSurface

  SDL_LockTextureToSurface(m_blurTexture, NULL, &m_blur);
  SetLightingRenderInfo();
  SDLProfilerStart->End();

  pixels = (uint32_t *)m_surface->pixels;

  WolfProfiler->Start();

  doneCount.store(0); // startBlur.store(true);
  startRender.store(true, std::memory_order_release);

  // Wait for all threads to finish
  while (doneCount.load() < NUM_THREADS)
  {

    std::this_thread::yield();
  }
  SpriteProfiler->Start();
  auto player = IEntitySystem->GetLocalPlayer();
  RenderSprites(player); // yk it might b cool to render the viewmodel at full res
  player->RenderView(this);
  SpriteProfiler->End();

  WolfProfiler->End();

  BlurProfiler->Start();
  if (m_bBlur)
  {
    
    if (BLUR_SCALE != 1)
      SDL_BlitSurfaceScaled(m_lightsurface, NULL, m_downscale, NULL); // SDL_SoftStretchLinear

    doneCount.store(0);
    startRender.store(false);
    startBlur.store(true);
    while (doneCount.load() < NUM_THREADS)
    {
      SDL_UnlockTexture(m_renderTexture);
      if (!m_isUpScaling )
        SDL_RenderTexture(get(), m_renderTexture, NULL, NULL); //do this while waiting for blur
      else
        SDL_RenderTexture(get(), m_renderTexture, NULL, &scale);
      std::this_thread::yield();
    }
    startBlur.store(false);  
    
  }

  BlurProfiler->End();
  SDLProfiler->Start();

  SDL_UnlockTexture(m_blurTexture); //expensive
  if(!m_bBlur)
  {
    if (!m_isUpScaling )
      SDL_RenderTexture(get(), m_renderTexture, NULL, NULL); //do this while waiting for blur
    else
      SDL_RenderTexture(get(), m_renderTexture, NULL, &scale);
  }
  

  if (!m_isUpScaling )
    SDL_RenderTexture(get(), m_blurTexture, NULL, NULL);
  else
    SDL_RenderTexture(get(), m_blurTexture, NULL, &scale);

  // R2::render_frame(this);
  RunImGui();

  SDL_RenderPresent(get());
  SDLProfiler->End();
}

void CRenderer::RunImGui()
{
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // ImGui::ShowDemoWindow();
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
