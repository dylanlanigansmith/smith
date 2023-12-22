#pragma once
#include <engine/engine.hpp>

#include "../CBaseEnemy.hpp"
#define IPOS_FIX 0.35f

namespace Enemy
{
    inline Vector2 FindPatrolPoint(const Vector2& pos, double min_dist = 6.0){
        Vector2 empty = ILevelSystem->FindEmptySpace();
        for(;;)
        {       
            empty = { empty.x + 0.4, empty.y + 0.4};
            if( (pos  - empty).LengthSqr()  > min_dist * min_dist && !ILevelSystem->GetTileAtFast(empty.x, empty.y)->Blocking() )
                break;
            empty = ILevelSystem->FindEmptySpace();
        }
        return empty;
    }

    inline Vector2 CenterPosition(const IVector2& i){
        return { i.x + IPOS_FIX, i.y + IPOS_FIX };
    }
    inline Vector2 FindRetreatPoint(const Vector2& pos, const Vector2& avoid, double min_dist = 6.0){
        Vector2 empty = ILevelSystem->FindEmptySpace();
        int bail = 0;
        for(;;)
        {       
            bail++;
            empty = { empty.x + 0.4, empty.y + 0.4};
            if( (avoid  - empty).LengthSqr()  > min_dist * min_dist )
                break;

            empty = ILevelSystem->FindEmptySpace();
            if(bail > 125){
                engine->warn("Enemy::FindRetreatPoint bailed");
                break;
            } 
        }
        return empty;
    }
}
