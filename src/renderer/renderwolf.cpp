#pragma GCC optimize ("O2")
#include "renderer.hpp"
#include <types/Vector.hpp>

#include <engine/engine.hpp>

#include <GL/gl.h>
#include <interfaces/interfaces.hpp>
#include <interfaces/IEngineTime/IEngineTime.hpp>
#include <util/misc.hpp>

/*
Goals :

modularize rendering 
alpha for thin walls

tile and texture flags 
-use voxels for col detection

timestep is so fucking broken

bonus if u add a settings mode

make lighting less of a shitshow

TRANSPARENCY

soo just make a queue of second passes
stdpair startx endx 

how will u break up render loop into funcs? beats me 



*/



void CRenderer::LoopWolf(int minX, int maxX)
{
  static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
  static auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
  static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
  static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
  // level system should handle these

  static int textH = 0, textW = 0;
  if(textH == 0 || textW == 0)
    ITextureSystem->GetTextureSize(&textW, &textH); //not even a good function

 // SDL_SetRenderTarget(get(), NULL);
 

  auto player = IEntitySystem->GetLocalPlayer();
  m_Camera = &player->m_camera;
  Vector playerPos =  player->GetPosition();


  int w = SCREEN_WIDTH; // so confusing
  int h = SCREEN_HEIGHT;
  IVector2 screen(w, h);

  DrawFloorCeiling(player, textW, textH, w, h,minX, maxX);

  // Draw Walls
  for (int x = minX; x < maxX; x++)
  {
    bool didDraw = false;
    Vector2 camera;
    camera.x = 2 * x / (double)screen.w() - 1.0;
    // calculate ray position and direction
    Vector2 rayDir = {
        m_Camera->m_vecDir.x + m_Camera->m_vecPlane.x * camera.x,
        m_Camera->m_vecDir.y + m_Camera->m_vecPlane.y * camera.x};

    // which box of the map we're in
    IVector2 mapPos(playerPos.x, playerPos.y);

    // length of ray from current position to next x or y-side
    Vector2 sideDist;

    // length of ray from one x or y-side to next x or y-side
    Vector2 deltaDist = {
        (rayDir.x == 0) ? 1e30 : std::abs(1 / rayDir.x),
        (rayDir.y == 0) ? 1e30 : std::abs(1 / rayDir.y)};

    double perpWallDist = 0.0, wallDistOffset = 0.0;

    IVector2 tex_offset = {0, 0};
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
    tile_t* hitTile = nullptr;
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
      auto tile = ILevelSystem->GetTileAt(mapPos.x, mapPos.y);
      if (!tile || tile == nullptr)
        continue;
      int type = tile->m_nType;

      if (type == Level::Tile_Wall){
         hit = 1; hitTile = tile;
      }
       

      if (tile->IsThinWall())
      {
        hit = 3;
        
        auto p = tile->m_vecPosition;
        auto wall = Render::GetLineForWallType(p, tile->m_nType, &side);
        Vector2 rayPos = {mapPos.x, mapPos.y};
        Vector2 startPos = playerPos;
        Ray_t ray = {
            .origin = startPos,
            .direction = rayDir.Normalize(),

        };
        Vector2 intersect;
        const double wall_thick = 0.2;
        BBoxAABB thickness = {
          .min = wall.p0,
          .max = wall.p1
        };

        if(side == 0)
          thickness.max = { wall.p1.x + wall_thick, wall.p1.y };
        else 
          thickness.max = { wall.p1.x , wall.p1.y + wall_thick };
        if (!Util::RayIntersectsLineSegment(ray, wall, intersect))
        {
          hit = 0;
          continue;
        }
        if(!Util::RayIntersectsBox(ray, thickness)) //todo add thickness
        {
          hit = 0; continue;
        }

        if (side == 0) {
        perpWallDist = (intersect.x - playerPos.x + (0) / 2)/ rayDir.x;
        } else {
            perpWallDist = (intersect.y - playerPos.y + (0) / 2) / rayDir.y;
        }
        // Calculate height of line to draw on screen
        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

        int pitch = m_Camera->m_flPitch;

        // calculate lowest and highest pixel to fill in current stripe
        int drawStart = -lineHeight / 2 + h / 2 + pitch + (playerPos.z / perpWallDist);
        if (drawStart < 0)
          drawStart = 0;
        int drawEnd = lineHeight / 2 + h / 2 + pitch + (playerPos.z / perpWallDist);
        if (drawEnd >= h)
          drawEnd = h - 1;

        // calculate value of wallX
        double wallX; // where exactly the wall was hit
       
         
        IVector2 tex;
        if (side == 1) {
        wallX = intersect.y - floor(intersect.y);
        } else {
            wallX = intersect.x - floor(intersect.x);
        }
        const float epsilon = 0.001f;  // Small threshold
     // wallX = intersect.y - floor(intersect.y);

      // Adjust for edge cases
      if (wallX < epsilon) {
        wallX = 0.0f;
      } else if (wallX > 1.0f - epsilon) {
        wallX = 1.0f;
      }// Apply texture mirroring if necessary
      // Convert wallX to texture coordinate
        tex.x = int(wallX * double(textW));
       // dbg("pDist%f %i mappos{%i %i} ist{%0.2f %0.2f} %f, %i", perpWallDist, side, mapPos.x, mapPos.y,intersect.x, intersect.y, wallX, tex.x); 
        double stepTex = 1.0 * textH / lineHeight;
        double texPos = (drawStart - pitch - h / 2 + lineHeight / 2) * stepTex;
        auto material = ILevelSystem->GetTextureAt(mapPos.x, mapPos.y);
        auto texture = material->m_texture;
        if(material->isTransparent()){
          //keep drawing -> need to break this into a function it is getting ridonk
        }
        auto tile = ILevelSystem->GetTileAt(mapPos);
        //bool hasBulletHole = (tile->m_nDecals > 0);
      
        // if(hasBulletHole) log("%i %i", mapPos.x, mapPos.y);
        for (int y = drawStart; y < drawEnd; y++)
        {
          // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
          tex.y = (int)texPos & (textH - 1);
          tex.x += tex_offset.x;
          tex.y += tex_offset.y;
          texPos += stepTex;

          uint32_t *pixelsT = (uint32_t *)texture->pixels;
          Color color = pixelsT[(texture->pitch / 4 * tex.y) + tex.x]; // ABGR
          if(color.a() == 0u) continue;
        
          //removed: bullet holes went here

          if (side == 1) // make color darker for y-sides
            color /= 0.5f;
          if(x > maxX) continue;

               //log("{%i %i} { %.1f,  %.1f,  %.4f}, %i %i ", mapPos.x, mapPos.y,wp.x,wp.y,wp.z);
          float lightH =  1.0 - ((float) (y - drawStart) /  (float) (drawEnd - drawStart ));
      
          Vector wp = Vector(intersect.x - (rayDir.x / STEPMAGICNUM), intersect.y - (rayDir.y / STEPMAGICNUM), lightH  );
          ILightingSystem->ApplyLightForTile(tile, (rayDir.x > 0), (rayDir.y > 0),wp, x, y);
          SetPixel(x,y, color);
          didDraw = true;
   
        }
      }
    }

    if(didDraw){
       ZBuffer[x] = perpWallDist; continue;
    }
    if(hitTile == nullptr) continue;
    auto tile = hitTile;
    // Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
    // hit to the camera plane. Euclidean to center camera point would give fisheye effect!
    // This can be computed as (mapX - player->GetPosition().x + (1 - stepX) / 2) / rayplayerDir.x for side == 0, or same formula with Y
    // for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
    // because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
    // steps, but we subtract deltaDist once because one step more into the wall was taken above.
    if (side == 0)
      perpWallDist = (sideDist.x - deltaDist.x) + wallDistOffset;
    else
      perpWallDist = (sideDist.y - deltaDist.y) + wallDistOffset;

    // Calculate height of line to draw on screen
    int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist) ;//
    

    int pitch = m_Camera->m_flPitch;

    // calculate lowest and highest pixel to fill in current stripe
    int drawStart = -lineHeight / 2 + h / 2 + pitch + (playerPos.z / perpWallDist);
    if (drawStart < 0)
      drawStart = 0;
    int drawEnd = lineHeight / 2 + h / 2 + pitch + (playerPos.z / perpWallDist);
    if (drawEnd >= h)
      drawEnd = h - 1;
 if(hitTile->m_flCeiling > 1.0f) drawStart -= hitTile->m_flCeiling * 15;
  if (drawStart < 0)
      drawStart = 0;
    // calculate value of wallX
    double wallX; // where exactly the wall was hit
    if (side == 0)
      wallX = playerPos.y + perpWallDist * rayDir.y;
    else
      wallX = playerPos.x + perpWallDist * rayDir.x;
    wallX -= floor((wallX));

    IVector2 tex;
    tex.x = int(wallX * double(textW));
    if (side == 0 && rayDir.x > 0)
      tex.x = textW - tex.x - 1;
    if (side == 1 && rayDir.y < 0)
      tex.x = textW - tex.x - 1;

    double stepTex = 1.0 * textH / lineHeight;
    double texPos = (drawStart - pitch - h / 2 + lineHeight / 2) * stepTex;
    auto texture = tile->m_pTexture->m_texture;//ILevelSystem->GetTextureAt(mapPos.x, mapPos.y)->m_texture;

   // auto tile = ILevelSystem->GetTileAt(mapPos);
    bool hasBulletHole = (tile->m_nDecals > 0);
  
    constexpr Color hole_color(0, 0, 0, 175);

   
    // if(hasBulletHole) log("%i %i", mapPos.x, mapPos.y);
    for (int y = drawStart; y < drawEnd; y++)
    {
      // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
      tex.y = (int)texPos & (textH - 1);
      tex.x += tex_offset.x;
      tex.y += tex_offset.y;
      texPos += stepTex;

      uint32_t *pixelsT = (uint32_t *)texture->pixels;
      Color color = pixelsT[(texture->pitch / 4 * tex.y) + tex.x]; // ABGR

      

      if (hasBulletHole && 22 < tex.y && tex.y < 42)
      {
        bool setPixel = false;
        auto pDecals = tile->m_pDecals;
        int i = 0;
        while (1 && pDecals != nullptr)
        {
          int hole_side = pDecals->side;
          uint8_t hole_x = pDecals->dir[0];
          uint8_t hole_y = pDecals->dir[1];
          if (side == hole_side && ((side == 0 && (step.x > 0) == hole_x) || (side == 1 && (step.y > 0) == hole_y)))
          {
            int radius = pDecals->radius;
            IVector2 &hole = pDecals->texturePosition;
            auto delta = hole - tex;
            if (radius * radius >= delta.x * delta.x + delta.y * delta.y + 0.25f)
            {
              // SetPixel(x,y,  Render::MergeColorsFixed( hole_color, color, hole_alpha, oalpha));
              SetPixel(x, y, hole_color + color);
              setPixel = true;
              break;
            }
          }
          i++;
          if (pDecals->m_pNextDecal == nullptr)
            break;
          pDecals = pDecals->m_pNextDecal;
        }
        if (setPixel)
          continue;
      }
      if (side == 1) // make color darker for y-sides
        color /= 0.5f;
     

     
      
        //log("{%i %i} { %.1f,  %.1f,  %.4f}, %i %i ", mapPos.x, mapPos.y,wp.x,wp.y,wp.z);
      float lightH =  1.0 - ((float) (y - drawStart) /  (float) (drawEnd - drawStart ));
      
      Vector wp = Vector((mapPos.x + wallX) - (rayDir.x / STEPMAGICNUM), mapPos.y - (rayDir.y / STEPMAGICNUM), lightH  );
      ILightingSystem->ApplyLightForTile(tile, (rayDir.x > 0), (rayDir.y > 0),wp, x, y);
      SetPixel(x,y, color);
     
    }
    ZBuffer[x] = perpWallDist;
  }
  if(minX > 200){
  //  m_bThreadDone = true;
    return;
  }
    

 RenderSprites(player);
  
}

void CRenderer::DrawFloorCeiling(CPlayer *player, const int textW, const int textH, const int w, const int h,int minX, int maxX)
{
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
   static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
  const double vertPos = 0.5; //
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
    float camZ = is_floor ? (vertPos * SCREEN_HEIGHT + player->GetPosition().z) : (vertPos * SCREEN_HEIGHT - player->GetPosition().z);

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
    float floorX = player->GetPosition().x + rowDistance * rayDirX0;
    float floorY = player->GetPosition().y + rowDistance * rayDirY0;

    for (int x = 0; x < w; ++x)
    {
      if(x < minX) {
        floorX += floorStepX;
      floorY += floorStepY;
        continue;
      
      }
      // the cell coord is simply got from the integer parts of floorX and floorY
      int cellX = (int)(floorX);
      int cellY = (int)(floorY);
      /*
      auto tile = ILevelSystem->GetTileSafe(cellX, cellY);
      if(tile->m_flCeiling != 0.5f && !is_floor){
          p =  (SCREEN_HEIGHT / 2 - y + m_Camera->m_flPitch);
          camZ =  (tile->m_flCeiling * SCREEN_HEIGHT - player->GetPosition().z);
          rowDistance = camZ / p;


           // real world coordinates of the leftmost column. This will be updated as we step to the right.
        floorX = player->GetPosition().x + rowDistance * rayDirX0;
        floorY = player->GetPosition().y + rowDistance * rayDirY0;
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
      auto tile = ILevelSystem->GetTileSafe(cellX, cellY);
     
      auto texture = ILevelSystem->GetTexturePlane(is_floor, cellX, cellY)->m_texture;
      
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

  // draw crosshair

  static constexpr int crosshair_x = SCREEN_WIDTH / 2;
  static constexpr int crosshair_y = SCREEN_HEIGHT / 2;
  static constexpr Color crosshair_color = Color::White();
  SetPixel(crosshair_x, crosshair_y,  crosshair_color);
  for (int x = crosshair_x - 4; x <= crosshair_x + 4; ++x)
    for (int y = crosshair_y - 1; y <= crosshair_y + 1; ++y)
      SetPixel( x, y,  crosshair_color);
  for (int y = crosshair_y - 4; y <= crosshair_y + 4; ++y)
    for (int x = crosshair_x - 1; x <= crosshair_x + 1; ++x)
      SetPixel(x, y,  crosshair_color);

}
