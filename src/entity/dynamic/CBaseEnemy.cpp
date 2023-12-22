#include "CBaseEnemy.hpp"
#include <engine/engine.hpp>

bool CBaseEnemy::m_ignoringPlayer = false;

bool CBaseEnemy::isEntityVisible(CBaseEntity *ent, double fov)
{
    if(ent->IsLocalPlayer() && m_ignoringPlayer) return false;
    if(isPointInFOV(ent->GetPosition(), fov))
        if(CastRayToPoint(ent->GetPosition(), ent->GetBounds()))
            return true;
    return false;
}

bool CBaseEnemy::isPointInFOV(const Vector2 &pos, double FOVAngleDegrees)
{
      Vector2 delta = pos - Vector2(GetPosition());
    double dot = m_view.m_dir.Normalize().dot(delta.Normalize());

    double halfFOVRadians = (FOVAngleDegrees / 2.0) * (M_PI / 180.0);
    double cosHalfFOV = cos(halfFOVRadians);
    return dot >= cosHalfFOV;
}

bool CBaseEnemy::CastRayToPoint(const Vector2 &pos, float bounds) //false = hit wall & true = reached pt unobstructed
{
     Ray_t ray = {
        .origin = Vector2(GetPosition()),
        .direction = (pos - GetPosition()).Normalize() 
    };
    if(Util::RayIntersectsCircle(ray, pos, bounds))
    {
        //engine->log("in circle");
        int hit = 0;
        Vector2 ray_pos = ray.origin;
        double step = 0.2;
        double dist_to_player = (pos - ray.origin).Length();
        IVector2 lastPos = {-1,-1};
        while((pos- ray_pos).Length() >= bounds)
        {
            //engine->log("CastRay %.3f %.3f", ray_pos.x, ray_pos.y);
            ray_pos = ray_pos + (ray.direction * step);
          
            if(ray_pos.x < 0 || ray_pos.y < 0) return false;
            if((int)floor(ray_pos.x) == lastPos.x && (int)floor(ray_pos.y) == lastPos.y)
                continue;
            lastPos = ray_pos;
            auto tile = ILevelSystem->GetTileAt(lastPos);
            if(tile->isEmpty()) continue;
            if(tile->Blocking()) 
                return false;
            
        }
        return true;
        //engine->log("cast ray hit player");
    }
    return false;
}
