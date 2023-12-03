#include "renderer.hpp"
#include <types/Vector.hpp>

#include <engine/engine.hpp>

#include <GL/gl.h>
#include <interfaces/interfaces.hpp>
#include <interfaces/IEngineTime/IEngineTime.hpp>


void CRenderer::LoopWolf()
{
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
  static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
  static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
  // level system should handle these
 

  int textH, textW;
  ITextureSystem->GetTextureSize(&textW, &textH);

  SDL_SetRenderTarget(get(), NULL);
  SDL_LockSurface(m_surface);
  pixels = (uint32_t *)m_surface->pixels;

  auto player = IEntitySystem->GetLocalPlayer();
  m_Camera = &player->m_camera;
  Vector2 playerPos = {
      player->GetPosition().x,
      player->GetPosition().y,
  };

  int w = SCREEN_WIDTH; // so confusing
  int h = SCREEN_HEIGHT;
  IVector2 screen(w, h);
  for (int y = 0; y < SCREEN_HEIGHT; ++y)
  {
    // whether this section is floor or ceiling
    bool is_floor = y > SCREEN_HEIGHT / 2 + player->Camera().m_flPitch;

    // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
    float rayDirX0 = player->Camera().m_vecDir.x - player->Camera().m_vecPlane.x;
    float rayDirY0 = player->Camera().m_vecDir.y - player->Camera().m_vecPlane.y;
    float rayDirX1 = player->Camera().m_vecDir.x + player->Camera().m_vecPlane.x;
    float rayDirY1 = player->Camera().m_vecDir.y + player->Camera().m_vecPlane.y;

    // Current y position compared to the center of the screen (the horizon)
    int p = is_floor ? (y - SCREEN_HEIGHT / 2 - player->Camera().m_flPitch) : (SCREEN_HEIGHT / 2 - y + player->Camera().m_flPitch);

    // Vertical position of the camera.
    // NOTE: with 0.5, it's exactly in the center between floor and ceiling,
    // matching also how the walls are being raycasted. For different values
    // than 0.5, a separate loop must be done for ceiling and floor since
    // they're no longer symmetrical.
    float camZ = is_floor ? (0.5 * SCREEN_HEIGHT + player->GetPosition().z) : (0.5 * SCREEN_HEIGHT - player->GetPosition().z);

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
    float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / SCREEN_WIDTH;
    float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / SCREEN_WIDTH;

    // real world coordinates of the leftmost column. This will be updated as we step to the right.
    float floorX = player->GetPosition().x + rowDistance * rayDirX0;
    float floorY = player->GetPosition().y + rowDistance * rayDirY0;

    for (int x = 0; x < SCREEN_WIDTH; ++x)
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
   
      auto texture = ILevelSystem->GetTexturePlane(is_floor, cellX, cellY)->m_texture;
     


      uint32_t *pixelsT = (uint32_t *)texture->pixels;
      uint32_t uColor = pixelsT[(texture->pitch / 4 * tex.y) + tex.x]; // ABGR

      SDL_Color color = Render::TextureToSDLColor(uColor);

      if (is_floor)
        Render::DarkenSDLColor(color, 2.f);
      else
        Render::DarkenSDLColor(color, 1.25f);

      int index = (y * m_surface->pitch / 4) + x;
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
      int type = ILevelSystem->GetMapAt(mapPos.x, mapPos.y);
      if (type != Level::Tile_Empty)
        hit = 1;

      if(type == Level::Tile_Door)
      {
        hit = 3;
      }

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
      int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

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
      auto texture = ILevelSystem->GetTextureAt(mapPos.x, mapPos.y)->m_texture;
     
      auto tile = ILevelSystem->GetTileAt(mapPos);
      bool hasBulletHole = (tile->m_nDecals > 0);
      const float hole_alpha = (175 / 255.f);
      const float oalpha = 1.f - hole_alpha;
      const SDL_Color hole_color = {0,0,0, 175};
    // if(hasBulletHole) log("%i %i", mapPos.x, mapPos.y);
      for (int y = drawStart; y < drawEnd; y++)
      {
        // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
        tex.y = (int)texPos & (textH - 1);
        texPos += stepTex;

        uint32_t *pixelsT = (uint32_t *)texture->pixels;
        uint32_t uColor = pixelsT[(texture->pitch / 4 * tex.y) + tex.x]; // ABGR

        SDL_Color color = Render::TextureToSDLColor(uColor);
        
        if(hasBulletHole && 22 < tex.y && tex.y < 42)
        {
          bool setPixel = false;
          auto pDecals = tile->m_pDecals;
          int i  = 0;
          while(1  && pDecals != nullptr){
            int hole_side = pDecals->side;
            uint8_t hole_x = pDecals->dir[0];
            uint8_t hole_y = pDecals->dir[1];
            if(side == hole_side &&  (  (side == 0 && (step.x > 0) == hole_x)  || (side == 1 && (step.y > 0) == hole_y ) ) ){
                int radius = pDecals->radius;
                IVector2& hole = pDecals->texturePosition;
                auto delta = hole - tex;
                if( radius*radius >= delta.x*delta.x + delta.y*delta.y + 0.25f  ){
                 // SetPixel(x,y,  Render::MergeColorsFixed( hole_color, color, hole_alpha, oalpha));
                 SetPixel(x,y,  Render::MergeColorsLazy( hole_color, color));  
                  setPixel = true; break;
                }
            }
            i++;
            if(pDecals->m_pNextDecal == nullptr) break;
            pDecals = pDecals->m_pNextDecal;
          }
          if(setPixel) continue;
        }
        if (side == 1) // make color darker for y-sides
            Render::DarkenSDLColor(color, 2.f);
        int index = (y * m_surface->pitch / 4) + x;
        pixels[index] = Render::SDLColorToWorldColor(color);
      
       /*
       so turns out transparency is as easy as bullet holes with mergecolorsfast 
       
       */
        
      }
      ZBuffer[x] = perpWallDist;
    

  }
  //draw renderable entities
  int numSprites = IEntitySystem->NumRenderables();
  std::vector<std::pair<double, uint32_t>> render_list;
  
  auto player_pos = player->GetPosition();
  for(auto ent  : IEntitySystem->iterableList() )
  {
    if(ent->IsRenderable() && !ent->IsLocalPlayer()){
      auto entPos = ent->GetPosition();
      double distanceSqr = Vector(player_pos - entPos).LengthSqr();
      render_list.push_back({distanceSqr, ent->GetID()});
    }
  }
  assert(numSprites == render_list.size()); //no longer needed
  std::sort(render_list.begin(), render_list.end(), 
        [](const std::pair<double, uint32_t> &a, const std::pair<double, uint32_t> &b) {
            return a.first > b.first; // Sorting in descending order based on the double value
        }
  );
  for(const auto& entry : render_list){
    auto ent = IEntitySystem->GetEntity<CBaseRenderable>(entry.second);
    if(!ent) continue;
    ent->Render(this);
  }
  //render localplayer stuff
  player->Render(this);


  //draw crosshair

  int crosshair_x = SCREEN_WIDTH / 2;
  int crosshair_y = SCREEN_HEIGHT / 2;
  SDL_Color crosshair_color = { 255, 255, 255, 225};
  Render::SetPixel(pixels, crosshair_x, crosshair_y, m_surface->pitch, crosshair_color);
  for(int x = crosshair_x - 4; x <= crosshair_x + 4; ++x)
    for(int y = crosshair_y - 1; y <= crosshair_y + 1; ++y)
      Render::SetPixel(pixels, x, y, m_surface->pitch, crosshair_color);
  for(int y = crosshair_y - 4; y <= crosshair_y + 4; ++y)
    for(int x = crosshair_x - 1; x <= crosshair_x + 1; ++x)
      Render::SetPixel(pixels, x, y, m_surface->pitch, crosshair_color);

  SDL_UnlockSurface(m_surface);
}
