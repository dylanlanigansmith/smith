#pragma GCC optimize ("O2")
#include "renderer.hpp"
#include <types/Vector.hpp>

#include <engine/engine.hpp>

#include <GL/gl.h>
#include <interfaces/interfaces.hpp>
#include <interfaces/IEngineTime/IEngineTime.hpp>
#include <util/misc.hpp>
#include "raycast.hpp"
/*
Goals :


rewrite resourcesystem for platform spec file sys -> use sdl stuff

tile and texture flags 
-use voxels for col detection, and pathfinding
also a flag for istransparent for 000 alpha clr 

make sure timestep isnt broken

overhaul entity system
- serialization based, callbacks, events

overhaul animations
- relative coords, seperate viewmodel

muzzle flash
lighting queue within pixel radius and ray dist gets a boost while anim plays
ez "dynamic" light
would be huge effect
  - register callback function for animation end!!

in general events and callbacks are gonna be needed soon
*/



void CRenderer::LoopWolf(int minX, int maxX, bool sprites)
{
  static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
  static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
  
  static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
  // level system should handle these

  static int textH = 0, textW = 0;
  if(textH == 0 || textW == 0)
    ITextureSystem->GetTextureSize(&textW, &textH); //not even a good function and nt used anympre

 // SDL_SetRenderTarget(get(), NULL);
 

  auto player = IEntitySystem->GetLocalPlayer();
  m_Camera = &player->m_camera;
  Vector playerPos =  player->GetPosition();

  auto tile = ILevelSystem->GetTileAt(playerPos.x, playerPos.y);
  const uint8_t type = (tile != nullptr) ? tile->m_nType : 0;

  DrawFloorCeiling(player, textW, textH, SCREEN_WIDTH, SCREEN_HEIGHT,minX, maxX);

  // Draw Walls
  for (int x = minX; x < maxX; x++)
  {
   
    Render::DrawLineAtX(x, type, this, m_Camera, playerPos, ILightingSystem, ILevelSystem); //only passing interfaces because its a pain to get them in a header 
  }

  /*
  if(minX > 200){
  //  m_bThreadDone = true;
    return;
  }*/
    
 //if(sprites)
  
  
}

void CRenderer::DrawFloorCeiling(CPlayer *player, const int textW, const int textH, const int w, const int h,int minX, int maxX)
{
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
   static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
  const double vertPos = 0.5; //
  const auto playerPos = player->GetPosition();
  for (int y = 0; y < SCREEN_HEIGHT; ++y)
  {
    // whether this section is floor or ceiling
    bool is_floor = y > SCREEN_HEIGHT / 2 + m_Camera->m_flPitch;

    // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
    float rayDirX0 = m_Camera->m_vecDir.x - m_Camera->m_vecPlane.x;
    float rayDirY0 = m_Camera->m_vecDir.y - m_Camera->m_vecPlane.y;
    float rayDirX1 = m_Camera->m_vecDir.x + m_Camera->m_vecPlane.x;
    float rayDirY1 = m_Camera->m_vecDir.y + m_Camera->m_vecPlane.y;

    // Current y position compared to the center of the screen (the horizon)
    int p = is_floor ? (y - SCREEN_HEIGHT / 2 - m_Camera->m_flPitch) : (SCREEN_HEIGHT / 2 - y + m_Camera->m_flPitch);
    
    // Vertical position of the camera.
    // NOTE: with 0.5, it's exactly in the center between floor and ceiling,
    // matching also how the walls are being raycasted. For different values
    // than 0.5, a separate loop must be done for ceiling and floor since
    // they're no longer symmetrical.
    float camZ = is_floor ? (vertPos * SCREEN_HEIGHT + playerPos.z) : (vertPos * SCREEN_HEIGHT - playerPos.z);

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
   // if(!is_floor) dbg("%i %f", p, rowDistance);

    // calculate the real world step vector we have to add for each x (parallel to camera plane)
    // adding step by step avoids multiplications with a weight in the inner loop
    float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / SCREEN_WIDTH;
    float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / SCREEN_WIDTH;

    // real world coordinates of the leftmost column. This will be updated as we step to the right.
    float floorX = playerPos.x + rowDistance * rayDirX0;
    float floorY = playerPos.y + rowDistance * rayDirY0;

    for (int x = 0; x < maxX; ++x)
    {
      if(x < minX) {
        floorX += floorStepX;
      floorY += floorStepY;
        continue;
      
      }
      // the cell coord is simply got from the integer parts of floorX and floorY
      int cellX = (int)(floorX);
      int cellY = (int)(floorY);
      
      auto tile = ILevelSystem->GetTileSafe(cellX, cellY);
      /*
      if(tile->m_flCeiling > 1.0 && !is_floor){
          p =  (SCREEN_HEIGHT / 2 - y + m_Camera->m_flPitch);
          camZ =  (tile->m_flCeiling * SCREEN_HEIGHT - playerPos.z);
          rowDistance = camZ / p;


           // real world coordinates of the leftmost column. This will be updated as we step to the right.
        floorX = playerPos.x + rowDistance * rayDirX0;
        floorY = playerPos.y + rowDistance * rayDirY0;
        cellX = (int)(floorX);
        cellY = (int)(floorY);
        floorStepX = rowDistance * (rayDirX1 - rayDirX0) / SCREEN_WIDTH;
        floorStepY = rowDistance * (rayDirY1 - rayDirY0) / SCREEN_WIDTH;
      }*/


      // get the texture coordinate from the fractional part
      IVector2 tex;
      tex.x = (int)(textW * (floorX - cellX)) & (textW - 1);
      tex.y = (int)(textH * (floorY - cellY)) & (textH - 1);

      floorX += floorStepX;
      floorY += floorStepY;

      // choose texture and draw the pixel
     // auto tile = ILevelSystem->GetTileSafe(cellX, cellY);
     
      auto texture = (is_floor) ? tile->m_pTextureFloor->m_texture : tile->m_pTextureCeiling->m_texture;
      //ILevelSystem->GetTexturePlane(is_floor, cellX, cellY)->m_texture;
      
      uint32_t *pixelsT = (uint32_t *)texture->pixels;
      Color color = pixelsT[(texture->pitch / 4 * tex.y) + tex.x]; // ABGR

       ILightingSystem->ApplyLightForTile(tile, (rayDirX0 > 0), (rayDirY0 > 0), {floorX , floorY , (is_floor) ? 0.15f : 0.9f }, x, y);
 
     // if(color == Color(0,255,255,255)) continue;
      if (is_floor)
        color /= 2.f;
      else
        color /= 1.25f;

      int index = (y * m_surface->pitch / 4) + x;
      pixels[index] = color;
    }
  }

  
}



void CRenderer::RenderSprites(CPlayer* player)
{
   static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
 
   // draw renderable entities
 // int numSprites = IEntitySystem->NumRenderables();
  std::vector<std::pair<double, CBaseRenderable*>> render_list;

  auto player_pos = player->GetPosition();
  for (auto ent : IEntitySystem->iterableList())
  {
    if (ent->IsRenderable() && !ent->IsLocalPlayer())
    {
      auto entPos = ent->GetPosition();
      double distanceSqr = Vector(player_pos - entPos).LengthSqr();
      render_list.push_back({distanceSqr, (CBaseRenderable*)ent});
    }
  }
  //assert(numSprites == render_list.size()); // this never was an issue
  std::sort(render_list.begin(), render_list.end(),
            [](const std::pair<double,CBaseRenderable*> &a, const std::pair<double, CBaseRenderable*> &b)
            {
              return a.first > b.first; // Sorting in descending order based on the double value
            });
  for (const auto &entry : render_list)
  {
    auto ent = entry.second;
    if (!ent)
      continue;
    ent->Render(this);
  }
  // render localplayer stuff
  player->Render(this);

  
}
