#include "renderer.hpp"

#include <types/Vector.hpp>

#include <engine/engine.hpp>

#include <GL/gl.h>
#include <interfaces/interfaces.hpp>
#include <interfaces/IEngineTime/IEngineTime.hpp>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_impl_sdl3.h>
#include "render_helpers.hpp"

void CRenderer::Shutdown()
{
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

     ImGui_ImplSDL3_InitForSDLRenderer(m_SDLWindow, get());
    ImGui_ImplSDLRenderer3_Init(get());
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
#define screenWidth 1024
#define screenHeight 720

void CRenderer::Loop()
{





  static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
  static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
  static hTexture hTextureBrick = ITextureSystem->LoadTexture("redbrick.png");
  static hTexture hBlueStone = ITextureSystem->LoadTexture("bluestone.png");
  static hTexture hColorStone = ITextureSystem->LoadTexture("bluestone.png");
  static hTexture hPurpleStone = ITextureSystem->LoadTexture("purplestone.png");
  auto textureBrick = ITextureSystem->GetTexture(hTextureBrick);
  auto textureBlueStone = ITextureSystem->GetTexture(hBlueStone);
  auto textureColorStone = ITextureSystem->GetTexture(hColorStone);
  auto texturePurpleStone = ITextureSystem->GetTexture(hPurpleStone);
  int textH = textureBrick->h;
  int textW = textureBrick->w;
  static auto renderText = SDL_CreateTexture(get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);

  static auto surf = SDL_CreateSurface(screenWidth, screenHeight, SDL_PIXELFORMAT_RGBA8888);
  SDL_SetRenderTarget(get(), NULL);
  SDL_LockSurface(surf);
  uint32_t *pixels = (uint32_t *)surf->pixels;

  int pitch = surf->pitch;

  auto player = IEntitySystem->GetLocalPlayer();
  Vector2 playerPos = {
      player->GetPosition().x,
      player->GetPosition().y,
  };

  for (int i = 0; i < 1280 * 720; ++i)
  {
    pixels[i] = (255 << 24u) | (0 << 16u) || (0 << 8u) | 255;
  }
  int w = screenWidth;
  int h = screenHeight;
  IVector2 screen(w, h);
  for (int y = 0; y < screenHeight; ++y)
  {
    // whether this section is floor or ceiling
    bool is_floor = y > screenHeight / 2 + player->Camera().m_flPitch;

    // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
    float rayDirX0 = player->Camera().m_vecDir.x - player->Camera().m_vecPlane.x;
    float rayDirY0 = player->Camera().m_vecDir.y - player->Camera().m_vecPlane.y;
    float rayDirX1 = player->Camera().m_vecDir.x + player->Camera().m_vecPlane.x;
    float rayDirY1 = player->Camera().m_vecDir.y + player->Camera().m_vecPlane.y;

    // Current y position compared to the center of the screen (the horizon)
    int p = is_floor ? (y - screenHeight / 2 - player->Camera().m_flPitch) : (screenHeight / 2 - y + player->Camera().m_flPitch);

    // Vertical position of the camera.
    // NOTE: with 0.5, it's exactly in the center between floor and ceiling,
    // matching also how the walls are being raycasted. For different values
    // than 0.5, a separate loop must be done for ceiling and floor since
    // they're no longer symmetrical.
    float camZ = is_floor ? (0.5 * screenHeight + player->GetPosition().z) : (0.5 * screenHeight - player->GetPosition().z);

    // Horizontal distance from the camera to the floor for the current row.
    // 0.5 is the z position exactly in the middle between floor and ceiling.
    // NOTE: this is affine texture mapping, which is not perspective correct
    // except for perfectly horizontal and vertical surfaces like the floor.
    // NOTE: this formula is explained as follows: The camera ray goes through
    // the following two points: the camera itself, which is at a certain
    // height (posZ), and a point in front of the camera (through an imagined
    // vertical plane containing the screen pixels) with horizontal distance
    // 1 from the camera, and vertical position p lower than posZ (posZ - p). When going
    // through that point, the line has vertically traveled by p units and
    // horizontally by 1 unit. To hit the floor, it instead needs to travel by
    // posZ units. It will travel the same ratio horizontally. The ratio was
    // 1 / p for going through the camera plane, so to go posZ times farther
    // to reach the floor, we get that the total horizontal distance is posZ / p.
    float rowDistance = camZ / p;

    // calculate the real world step vector we have to add for each x (parallel to camera plane)
    // adding step by step avoids multiplications with a weight in the inner loop
    float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / screenWidth;
    float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / screenWidth;

    // real world coordinates of the leftmost column. This will be updated as we step to the right.
    float floorX = player->GetPosition().x + rowDistance * rayDirX0;
    float floorY = player->GetPosition().y + rowDistance * rayDirY0;

    for (int x = 0; x < screenWidth; ++x)
    {
      // the cell coord is simply got from the integer parts of floorX and floorY
      int cellX = (int)(floorX);
      int cellY = (int)(floorY);

      // get the texture coordinate from the fractional part
      IVector2 tex;
      tex.x = (int)(textW * (floorX - cellX)) & (textW - 1);
      tex.y = (int)(textH * (floorY - cellY)) & (textH - 1);

      floorX += floorStepX;
      floorY += floorStepY;

      // choose texture and draw the pixel
      int checkerBoardPattern = (int(cellX + cellY)) & 1;
      auto texture = textureColorStone;
      if (checkerBoardPattern == 0 && is_floor)
        texture = textureBlueStone;
      else if(is_floor)
        texture = texturePurpleStone;        

     
      
      uint32_t *pixelsT = (uint32_t *)texture->pixels;
      uint32_t uColor = pixelsT[(texture->pitch / 4 * tex.y) + tex.x ] ;       //ABGR

      SDL_Color color = Render::TextureToSDLColor(uColor);
      
      if (is_floor)
        Render::DarkenSDLColor(color, 2.f);
      else Render::DarkenSDLColor(color, 1.25f);

     int index = (y * surf->pitch / 4) + x;
      pixels[index] = Render::SDLColorToWorldColor(color);
    }
  }

  // Draw Walls
  for (int x = 0; x < w; x++)
  {
    Vector2 camera;
    camera.x = 2 * x / (double)screen.w() - 1.0;
    // calculate ray position and direction
    Vector2 rayDir = {
        player->Camera().m_vecDir.x + player->Camera().m_vecPlane.x * camera.x,
        player->Camera().m_vecDir.y + player->Camera().m_vecPlane.y * camera.x};

    // which box of the map we're in
    IVector2 mapPos(playerPos.x, playerPos.y);

    // length of ray from current position to next x or y-side
    Vector2 sideDist;

    // length of ray from one x or y-side to next x or y-side
    Vector2 deltaDist = {
        (rayDir.x == 0) ? 1e30 : std::abs(1 / rayDir.x),
        (rayDir.y == 0) ? 1e30 : std::abs(1 / rayDir.y)};

    double perpWallDist;

    // what direction to step in x or y-direction (either +1 or -1)
    IVector2 step;

    int hit = 0; // was there a wall hit?
    int side;    // was a NS or a EW wall hit?
    if (rayDir.x < 0)
    {
      step.x = -1;
      sideDist.x = (playerPos.x - mapPos.x) * deltaDist.x;
    }
    else
    {
      step.x = 1;
      sideDist.x = (mapPos.x + 1.0 - playerPos.x) * deltaDist.x;
    }
    if (rayDir.y < 0)
    {
      step.y = -1;
      sideDist.y = (playerPos.y - mapPos.y) * (deltaDist.y);
    }
    else
    {
      step.y = 1;
      sideDist.y = (mapPos.y + 1.0 - playerPos.y) * (deltaDist.y);
    }
    // perform DDA
    while (hit == 0)
    {
      // jump to next map square, either in x-direction, or in y-direction
      if (sideDist.x < sideDist.y)
      {
        sideDist.x += deltaDist.x;
        mapPos.x += step.x;
        side = 0;
      }
      else
      {
        sideDist.y += deltaDist.y;
        mapPos.y += step.y;
        side = 1;
      }
      // Check if ray has hit a wall
      if (ILevelSystem->GetMapAt(mapPos.x, mapPos.y) > 0)
        hit = 1;
    }
    // Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
    // hit to the camera plane. Euclidean to center camera point would give fisheye effect!
    // This can be computed as (mapX - player->GetPosition().x + (1 - stepX) / 2) / rayplayerDir.x for side == 0, or same formula with Y
    // for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
    // because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
    // steps, but we subtract deltaDist once because one step more into the wall was taken above.
    if (side == 0)
      perpWallDist = (sideDist.x - deltaDist.x);
    else
      perpWallDist = (sideDist.y - deltaDist.y);

    // Calculate height of line to draw on screen
    int lineHeight = (int)(screenHeight / perpWallDist);

    int pitch = player->Camera().m_flPitch;

    // calculate lowest and highest pixel to fill in current stripe
    int drawStart = -lineHeight / 2 + h / 2 + pitch + (player->GetPosition().z / perpWallDist);
    if (drawStart < 0)
      drawStart = 0;
    int drawEnd = lineHeight / 2 + h / 2 + pitch + (player->GetPosition().z / perpWallDist);
    if (drawEnd >= h)
      drawEnd = h - 1;

    


    // calculate value of wallX
    double wallX; // where exactly the wall was hit
    if (side == 0)
      wallX = player->GetPosition().y + perpWallDist * rayDir.y;
    else
      wallX = player->GetPosition().x + perpWallDist * rayDir.x;
    wallX -= floor((wallX));

    IVector2 tex;
    tex.x = int(wallX * double(textW));
    if (side == 0 && rayDir.x > 0)
      tex.x = textW - tex.x - 1;
    if (side == 1 && rayDir.y < 0)
      tex.x = textW - tex.x - 1;

    double stepTex = 1.0 * textH / lineHeight;
    double texPos = (drawStart - pitch - h / 2 + lineHeight / 2) * stepTex;
    for (int y = drawStart; y < drawEnd; y++)
    {
      // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
      tex.y = (int)texPos & (textH - 1);
      texPos += stepTex;
   
      uint32_t *pixelsT = (uint32_t *)textureBrick->pixels;
      uint32_t uColor = pixelsT[(textureBrick->pitch / 4 * tex.y) + tex.x ] ;       //ABGR

      SDL_Color color = Render::TextureToSDLColor(uColor);
      // make color darker for y-sides
      if (side == 1)
        Render::DarkenSDLColor(color, 2.f);
      int index = (y * surf->pitch / 4) + x;
      pixels[index] = Render::SDLColorToWorldColor(color);
     
    }
  }
  SDL_UnlockSurface(surf);
  SDL_UpdateTexture(renderText, NULL, surf->pixels, surf->pitch);

  SDL_RenderTexture(get(), renderText, NULL, NULL);
 
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  //bool open = true;
  //ImGui::ShowDemoWindow(&open);

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());
  SDL_RenderPresent(get());
}
