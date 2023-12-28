#pragma once

#include "CBaseWeapon.hpp"
#include <entity/dynamic/CBaseEnemy.hpp>
#include <engine/engine.hpp>

struct hit_trace_data
{
    //dda stuff 
    int hit, side;
    Ray_t ray;
    Vector2 rayDir; //for DDA bc we dont normalize
    IVector2 step;
    Vector2 deltaDist; 
    Vector2 sideDist;
    IVector2 mapPos;

    CCamera* cam;

    hit_trace_data(CPlayer* player) //std dda setup
    {
        hit = 0; 
        cam = player->m_pCamera();
        rayDir = cam->CalcDDARayDir();
        ray.origin = player->GetPosition();
        ray.direction = rayDir.Normalize(); //not 100% necessary in most cases 
        mapPos = { ray.origin.x, ray.origin.y};

        deltaDist = {
            (rayDir.x == 0) ? 1e30 : std::abs(1 / rayDir.x),
            (rayDir.y == 0) ? 1e30 : std::abs(1 / rayDir.y)
        };
        if (rayDir.x < 0){
            step.x = -1;
            sideDist.x = (ray.origin.x - mapPos.x) * deltaDist.x;
        }
        else{
            step.x = 1;
            sideDist.x = (mapPos.x + 1.0 - ray.origin.x) * deltaDist.x;
        }
        if (rayDir.y < 0){
            step.y = -1;
            sideDist.y = (ray.origin.y - mapPos.y) * (deltaDist.y);
        }
        else{
            step.y = 1;
            sideDist.y = (mapPos.y + 1.0 - ray.origin.y) * (deltaDist.y);
        }

    }

    inline void JumpToNextTile(){
        if (sideDist.x < sideDist.y){
            sideDist.x += deltaDist.x;
            mapPos.x += step.x;
            side = 0;
        }
        else{
            sideDist.y += deltaDist.y;
            mapPos.y += step.y;
            side = 1;
        }
    }

    inline double CalcPerpWallDist(){
         if (side == 0)
            return (sideDist.x - deltaDist.x);
        else
            return (sideDist.y - deltaDist.y);
    }
};
namespace WeaponServices
{
    inline CBaseEnemy* CheckTileForHit(tile_t* tile, CCamera* cam, IVector2& textpos, const IVector2& xhair = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2})
    {
        if(!tile) return nullptr;

        if (!tile->m_occupants.empty()) //player doesnt get tiled
        {
            for (auto &id : tile->m_occupants)
            {
                auto ent = IEntitySystem->GetEntity(id);
                if (ent == nullptr)
                    continue;
                if (ent->IsLocalPlayer())
                    continue;
                if ( ent->IsEnemy())
                {
                    auto hit_ent = dynamic_cast<CBaseEnemy *>(ent);
             
                    if( hit_ent->HitDetect(cam, xhair, &textpos)){ //should return position
                        //caller must run OnHit 

                        return hit_ent;
                    }
                }

            }
        }


        return nullptr;
    }
};