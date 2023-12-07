#pragma once

#include <common.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>
#include <data/level.hpp>
#include <types/Color.hpp>
#include <interfaces/interfaces.hpp>
#include <entity/player/CPlayer.hpp>
#include <iostream>
struct raycast_data_t
{
    const Vector playerPos;
    const int playerPitch;
    const Vector2 rayDir;
    IVector2 step; // what direction to step in x or y-direction (either +1 or -1)
    IVector2 mapPos;
    Vector2 sideDist; // length of ray from current position to next x or y-side
    Vector2 deltaDist;  // length of ray from one x or y-side to next x or y-side
    double perpDist = 0.0;
    int side = -1;  // was a NS or a EW wall hit?
    tile_t* hitTile = nullptr;
    
    //prevent copying
    raycast_data_t( raycast_data_t const&) = delete;
    raycast_data_t& operator=(raycast_data_t const&) = delete;


    inline raycast_data_t(const Vector& playerPos,  const Vector2& rayDir, int playerPitch = 0) : 
       playerPos(playerPos), rayDir(rayDir),  playerPitch(playerPitch), side(-1), hitTile(nullptr){

        deltaDist = {
            (rayDir.x == 0) ? 1e30 : std::abs(1 / rayDir.x),
            (rayDir.y == 0) ? 1e30 : std::abs(1 / rayDir.y)};
        mapPos = IVector2(playerPos.x, playerPos.y);
    }
};

struct raycast_draw_t
{
    const IVector2 texture_size;
    IVector2 texture_pos; //y will be unset
    int drawStart; //y
    int drawEnd; //y
    int lineHeight;
    double wallX; // where exactly the wall was hit
    double textureStep;
    double textureY;
    
    

     //prevent copying
    raycast_draw_t( raycast_draw_t const&) = delete;
    raycast_draw_t& operator=(raycast_draw_t const&) = delete;

    inline raycast_draw_t(const IVector2& texture_size) : texture_size(texture_size){}


};

namespace Render
{

    



    inline void perpDist_Wall(raycast_data_t& ray){
        /*note from raycaster tutorial
            // Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
    // hit to the camera plane. Euclidean to center camera point would give fisheye effect!
    // This can be computed as (mapX - player->GetPosition().x + (1 - stepX) / 2) / rayplayerDir.x for side == 0, or same formula with Y
    // for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
    // because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
    // steps, but we subtract deltaDist once because one step more into the wall was taken above.
        */
      
       
        if(ray.side == 0)
            ray.perpDist = (ray.sideDist.x - ray.deltaDist.x);
        else
            ray.perpDist = (ray.sideDist.y - ray.deltaDist.y);

          
    }
    inline void perpDist_ThinWall(raycast_data_t& ray, const Vector2& intersect){
        if (ray.side == 0) {
            ray.perpDist = (intersect.x - ray.playerPos.x )/ ray.rayDir.x;
        } 
        else {
            ray.perpDist = (intersect.y - ray.playerPos.y ) / ray.rayDir.y;
        }
    }

    inline void CalcDrawHeightBounds(raycast_data_t& ray, raycast_draw_t& draw)
    {
        draw.lineHeight = (int)(SCREEN_HEIGHT / ray.perpDist) ; // Calculate height of line to draw on screen
         // calculate lowest and highest pixel to fill in current stripe
        draw.drawStart = -draw.lineHeight / 2 + SCREEN_HEIGHT / 2 + ray.playerPitch + (ray.playerPos.z / ray.perpDist);
        if (draw.drawStart < 0)
            draw.drawStart = 0;

        draw.drawEnd = draw.lineHeight / 2 + SCREEN_HEIGHT / 2 + ray.playerPitch + (ray.playerPos.z / ray.perpDist);
        if (draw.drawEnd >= SCREEN_HEIGHT)
            draw.drawEnd = SCREEN_HEIGHT - 1;
    }
    inline void GetTexturePosition_ThinWall(raycast_data_t& ray, raycast_draw_t& draw, const Vector2& intersect)
    {
        if (ray.side == 1) {
            draw.wallX  = intersect.y - floor(intersect.y);
        }
         else {
            draw.wallX = intersect.x - floor(intersect.x);
        }
        const float epsilon = 0.001f; 
        if (draw.wallX < epsilon) {
            draw.wallX = 0.0f;
        } else if (draw.wallX > 1.0f - epsilon) {
            draw.wallX = 1.0f;
        }
        //@todo: revisit this 
        draw.texture_pos.x = int(draw.wallX * double(draw.texture_size.x));
     
        draw.textureStep = 1.0 * draw.texture_size.y / draw.lineHeight;
        draw.textureY = (draw.drawStart - ray.playerPitch - (SCREEN_HEIGHT / 2) + (draw.lineHeight / 2) ) * draw.textureStep;
    }
    inline void GetTexturePosition_Wall(raycast_data_t& ray, raycast_draw_t& draw)
    {
        if (ray.side == 0)
            draw.wallX = ray.playerPos.y + ray.perpDist * ray.rayDir.y;
        else
            draw.wallX = ray.playerPos.x + ray.perpDist * ray.rayDir.x;
        draw.wallX -= floor((draw.wallX));

        draw.texture_pos.x = int(draw.wallX * double(draw.texture_size.x));
        if (ray.side == 0 && ray.rayDir.x > 0)
            draw.texture_pos.x = draw.texture_size.x - draw.texture_pos.x - 1;
        if (ray.side == 1 && ray.rayDir.y < 0)
            draw.texture_pos.x = draw.texture_size.x - draw.texture_pos.x - 1;
        
        draw.textureStep = 1.0 * draw.texture_size.y / draw.lineHeight;
        draw.textureY = (draw.drawStart - ray.playerPitch - SCREEN_HEIGHT / 2 + draw.lineHeight / 2) * draw.textureStep ;
    }


    inline void CalculateDrawData_Wall(raycast_data_t& ray, raycast_draw_t& draw)
    { 
        perpDist_Wall(ray);
        CalcDrawHeightBounds(ray, draw);
        GetTexturePosition_Wall(ray, draw);
    }


    inline void UpdateTextureCoords_Wall(raycast_draw_t& draw){
         // cast 2 int & mask with (texHeight - 1) in case of overflow
        draw.texture_pos.y = (int)draw.textureY & (draw.texture_size.y- 1);
        draw.textureY += draw.textureStep;
    }
    inline void UpdateTextureCoords_ThinWall(raycast_draw_t& draw){
         // cast 2 int & mask with (texHeight - 1) in case of overflow
        draw.texture_pos.y = (int)draw.textureY & (draw.texture_size.y- 1);
        draw.textureY += draw.textureStep;
    }

    inline void FindSideDistAndStep(raycast_data_t& ray ){
        if (ray.rayDir.x < 0){
            ray.step.x = -1;
            ray.sideDist.x = (ray.playerPos.x - ray.mapPos.x) * ray.deltaDist.x;
        }
        else{
            ray.step.x = 1;
            ray.sideDist.x = (ray.mapPos.x + 1.0 - ray.playerPos.x) * ray.deltaDist.x;
        }
        if (ray.rayDir.y < 0){
            ray.step.y = -1;
            ray.sideDist.y = (ray.playerPos.y - ray.mapPos.y) * (ray.deltaDist.y);
        }
        else{
            ray.step.y = 1;
            ray.sideDist.y = (ray.mapPos.y + 1.0 - ray.playerPos.y) * (ray.deltaDist.y);
        }
        
    }

    inline int RunDDA(raycast_data_t& ray, CLevelSystem*& ILevelSystem){
        
        int hit = 0;
    
        // jump to next map square, either in x-direction, or in y-direction
        if (ray.sideDist.x < ray.sideDist.y)
        {
            ray.sideDist.x += ray.deltaDist.x;
            ray.mapPos.x += ray.step.x;
            ray.side = 0;
        }
        else
        {
            ray.sideDist.y += ray.deltaDist.y;
            ray.mapPos.y += ray.step.y;
            ray.side = 1;
        }
        // Check if ray has hit a wall
        auto tile = ILevelSystem->GetTileAt(ray.mapPos);
        if (tile == nullptr)
            return 0; //we really should break here 
        int type = tile->m_nType;

        if (type != Level::Tile_Empty){
            hit = type; ray.hitTile = tile; 
        }
   
        return hit; //literally always going to hit
    }

    inline void SetupRaycastWall(raycast_data_t& ray)
    {
        FindSideDistAndStep(ray);
    }

    inline float CalculateHeightForLighting(const raycast_draw_t& draw, int y){
        return 1.f - ((float) (y - draw.drawStart) /  (float) (draw.drawEnd - draw.drawStart ));
    }

    inline Vector WorldPositionFromDrawInfo(const raycast_data_t& ray, const raycast_draw_t& draw, int y, const float magic = 8.f){
        return Vector(
            (ray.mapPos.x + draw.wallX) - (ray.rayDir.x / magic),
            (ray.mapPos.y) - (ray.rayDir.y / magic),
             CalculateHeightForLighting(draw, y)
        );
    }


    inline void DrawLineAtX(int x, uint8_t playerTileType, CRenderer* renderer, CCamera* m_Camera, const Vector& playerPos, CLightingSystem* ILightingSystem, CLevelSystem* ILevelSystem,  uint32_t skipTransparent = 0)
    {

        const int textW = 64, textH = 64;
        bool didDraw = false;
        Vector2 camera;
        camera.x = 2 * x / (double)SCREEN_WIDTH - 1.0;
        // calculate ray position and direction
        Vector2 rayDir = {
            m_Camera->m_vecDir.x + m_Camera->m_vecPlane.x * camera.x,
            m_Camera->m_vecDir.y + m_Camera->m_vecPlane.y * camera.x};
    
        raycast_data_t ray(playerPos, rayDir);
    
        Render::SetupRaycastWall(ray);
 
        int hit = 0; // was there a wall hit?
       
        // perform DDA
        uint32_t transparentHits = 0;
        int safety = 0;
        while (hit == 0)
        {
             safety++;
             if(safety > 250){
                engine->Error("hit DDA safety {%i %i}", ray.mapPos.x, ray.mapPos.y); return;
            }
            hit = Render::RunDDA(ray, ILevelSystem);
            if(ray.hitTile == nullptr) continue; // we got bigger issues
            if (hit == 0) continue;
            if (hit >= Level::Tile_Door )
            {
                auto& tile = ray.hitTile;

              
                auto p = tile->m_vecPosition;
                auto wall = Render::GetLineForWallType(p, tile->m_nType, &ray.side);
                Vector2 rayPos = {ray.mapPos.x,ray.mapPos.y};
                Vector2 startPos = playerPos;
                Ray_t ray1 = {
                    .origin = startPos,
                    .direction = rayDir.Normalize(),

                };
                Vector2 intersect;
                const double wall_thick = 0.9;
                BBoxAABB thickness = {
                .min = wall.p0,
                .max = wall.p1
                };

                if(ray.side == 0)
                thickness.max = { wall.p1.x + wall_thick, wall.p1.y };
                else 
                thickness.max = { wall.p1.x , wall.p1.y + wall_thick };
                if (!Util::RayIntersectsLineSegment(ray1, wall, intersect))
                {
                   
                    hit = 0; 
                    continue;
                }
                if(!Util::RayIntersectsBox(ray1, thickness)) //todo add thickness
                {   
                    hit = 0; continue;
                }
                auto material = ILevelSystem->GetTextureAt(ray.mapPos.x, ray.mapPos.y); //just use tile 
                auto texture = material->m_texture;
                if(material->isTransparent())
                {
                  
                    transparentHits++;
                    if(transparentHits >  skipTransparent)
                    {
                        // engine->log("out rip           ");
                        DrawLineAtX(x, 0, renderer, m_Camera, playerPos, ILightingSystem, ILevelSystem, skipTransparent + 1);
                    }
                    else{
                        //making 
                         hit = 0; continue;
                    }
                }
                
          
                
                Render::perpDist_ThinWall(ray, intersect);
                auto draw = raycast_draw_t({textW, textH});
                
                Render::CalcDrawHeightBounds(ray, draw);
                Render::GetTexturePosition_ThinWall(ray, draw, intersect);
            
                
            
            
                for (int y = draw.drawStart; y < draw.drawEnd; y++)
                {
                Render::UpdateTextureCoords_ThinWall(draw);

                uint32_t *pixelsT = (uint32_t *)texture->pixels;
                Color color = pixelsT[(texture->pitch / 4 * draw.texture_pos.y) + draw.texture_pos.x]; // ABGR
                if(color.a() == 0u) continue;
                
                //removed: bullet holes went here

                if (ray.side == 1) // make color darker for y-sides
                    color /= 0.5f;
                

                    //log("{%i %i} { %.1f,  %.1f,  %.4f}, %i %i ", mapPos.x, mapPos.y,wp.x,wp.y,wp.z);
                
            
                Vector wp = Render::WorldPositionFromDrawInfo(ray, draw, y, STEPMAGICNUM);
                ILightingSystem->ApplyLightForTile(tile, (ray.rayDir.x > 0), (ray.rayDir.y > 0),wp, x, y);
                renderer->SetPixel(x,y, color);
                if(material->isTransparent())
                    renderer->Z2D[x][y] = 1;
                else
                    renderer->Z2D[x][y] = 0; //sorta works, merging them into z buffer is probably the nicer option but the recursive thing works sooo well fuck
                didDraw = true;
        
                }
            }
            
        }

        if(didDraw){
            if(!ray.hitTile->m_pTexture->isTransparent())
                renderer->SetZBuffer(x, ray.perpDist);  
         //   else
           //     renderer->SetZBuffer(x, ray.perpDist - 1.0);  
            return;
        }
        if(ray.hitTile == nullptr) return;
        auto& tile = ray.hitTile;
        auto draw = raycast_draw_t({textW, textH});
        Render::CalculateDrawData_Wall(ray, draw);

    
        auto texture = tile->m_pTexture->m_texture;//ILevelSystem->GetTextureAt(mapPos.x, mapPos.y)->m_texture;

    // auto tile = ILevelSystem->GetTileAt(mapPos);
        bool hasBulletHole = (tile->m_nDecals > 0);
    
        static constexpr Color hole_color(0, 0, 0, 175);

    
    
        for (int y = draw.drawStart; y < draw.drawEnd; y++)
        {
        Render::UpdateTextureCoords_Wall(draw);

        uint32_t *pixelsT = (uint32_t *)texture->pixels;
        Color color = pixelsT[(texture->pitch / 4 * draw.texture_pos.y) + draw.texture_pos.x]; // ABGR

        

        if (hasBulletHole && 22 < draw.texture_pos.y && draw.texture_pos.y < 42)
        {
            bool setPixel = false;
            auto pDecals = tile->m_pDecals;
            int i = 0;
            while (1 && pDecals != nullptr)
            {
            int hole_side = pDecals->side;
            uint8_t hole_x = pDecals->dir[0];
            uint8_t hole_y = pDecals->dir[1];
            if (ray.side == hole_side && ((ray.side == 0 && (ray.step.x > 0) == hole_x) || (ray.side == 1 && (ray.step.y > 0) == hole_y)))
            {
                int radius = pDecals->radius;
                IVector2 &hole = pDecals->texturePosition;
                auto delta = hole - draw.texture_pos;
                if (radius * radius >= delta.x * delta.x + delta.y * delta.y + 0.25f)
                {
                // SetPixel(x,y,  Render::MergeColorsFixed( hole_color, color, hole_alpha, oalpha));
                renderer->SetPixel(x, y, hole_color + color);
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
        if (ray.side == 1) // make color darker for y-sides
            color /= 0.5f;
        
            //log("{%i %i} { %.1f,  %.1f,  %.4f}, %i %i ", mapPos.x, mapPos.y,wp.x,wp.y,wp.z);
        Vector wp = Render::WorldPositionFromDrawInfo(ray, draw, y, STEPMAGICNUM);
        ILightingSystem->ApplyLightForTile(tile, (ray.rayDir.x > 0), (ray.rayDir.y > 0),wp, x, y);
        renderer->SetPixel(x,y, color);
            renderer->Z2D[x][y] = 0;
        }
        renderer->SetZBuffer(x, ray.perpDist);
    }
}