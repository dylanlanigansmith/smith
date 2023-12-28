#include "CBaseWeapon.hpp"
#include <engine/engine.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>

#include <entity/level/CBaseDoorControl.hpp>
#include "weaponservices.hpp"


bool CBaseWeapon::Shoot() //returns true if a shot was fired!
{

    Debug(false);

    static auto owner = static_cast<CPlayer *>(m_pOwner); //this will cause a bug 
    auto curTick = IEngineTime->GetCurLoopTick();

    static looptick_t click = 0; //bad!
    if (m_nNextShot > curTick || m_clip == 0 || m_nNextReload > curTick){
        if(m_clip == 0 && click < curTick){
            click = curTick + TICKS_PER_S / 2; engine->SoundSystem()->PlaySound("empty-gun", 1.0);
        }
            
        return false; 
    }
        
    if(m_nNextShot - m_nFireRate == curTick - m_nFireRate ){
        m_shotsFired++; //should be a range, works okay though
        //also should have a tick cooldown to reset
    }
    else m_shotsFired = 0;
    m_nNextShot = curTick + m_nFireRate;
    m_clip --;
    
    this->OnShoot();
    this->ApplyRecoil();
    
    auto cam = owner->m_pCamera();
    auto pos = owner->GetPosition();
    
    static const IVector2 xhair = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

    //collision detection REWRITE THIS! holy fuck
    

    //check our own tile first 
    auto our_tile = ILevelSystem->GetTileAt(IVector2::Rounded(pos.x, pos.y)); //do we want rounded??
    IVector2 textpos;
    CBaseEnemy *hit_ent = WeaponServices::CheckTileForHit(our_tile, cam, textpos, xhair);
    if (hit_ent != nullptr) 
    {
        dbg("hit");   
        hit_ent->OnHit(GetDamage(), textpos); //damage isnt real!!!
        return true;
    }

    hit_trace_data data(owner);

    // perform DDA
    while (data.hit == 0)
    {
        
        data.JumpToNextTile();
     
        auto tile = ILevelSystem->GetTileAt(data.mapPos);
        
        CBaseEnemy *hit_ent = WeaponServices::CheckTileForHit(tile, cam, textpos, xhair);
        if (hit_ent != nullptr) 
        {
            dbg("hit");
        
            hit_ent->OnHit(GetDamage(), textpos); //damage isnt real!!!

            return true;
        }
        if(tile->IsThinWall() && tile->HasState()){
            if(tile->m_pState->m_isDoor && !tile->m_pState->m_doorctl->IsOpen())
            {
                auto wall = Render::GetLineForWallType(tile->m_vecPosition, tile->m_nType);
                Vector2 intersection;
                Ray_t ray = {
                    .origin = pos,
                    .direction = data.rayDir.Normalize()
                };
                if (Util::RayIntersectsLineSegment(ray, wall, intersection))
                {
                    tile->m_pState->m_doorctl->OnHit(GetDamage());
                    break; //really can just return true
                }
            }
        }

        // Check if ray has hit a wall we can leave a hole in
        if (tile->m_nType == Level::Tile_Wall )
            data.hit = 1;
    }
    // FOR BULLET HOLES ^

    if (data.hit)
    {
        double perpWallDist = data.CalcPerpWallDist();


        int lineHeight = (SCREEN_HEIGHT / perpWallDist);
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2 + cam->m_flPitch + (pos.z / perpWallDist);
        if(drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2 + cam->m_flPitch + (pos.z / perpWallDist);
        if(drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;
        IVector2 tex;
        //tex pos X
        double wallX; 
        if (data.side == 0)
            wallX = pos.y + perpWallDist * data.rayDir.y;
        else
            wallX = pos.x + perpWallDist * data.rayDir.x;
        wallX -= floor((wallX));
        
        constexpr int textW = 64, textH = 64; //bad

        tex.x = int(wallX * double(textW));
        if (data.side == 0 && data.rayDir.x > 0)
            tex.x = textW - tex.x - 1;
        if (data.side == 1 && data.rayDir.y < 0)
            tex.x = textW - tex.x - 1;

        double texStep = (double)textH / (double)lineHeight;
        double textureY = (drawStart - cam->m_flPitch - SCREEN_HEIGHT / 2 + lineHeight / 2 - (pos.z / perpWallDist)) * texStep;

        int y_off = xhair.y - drawStart;
        if(y_off <= 0) return true; //32 + Util::SemiRandRange(0, 24) - 12; //old way

        textureY += (double)y_off * texStep;
        tex.y = (int)textureY & (textH - 1);
       // tex.y = 32 + Util::SemiRandRange(0, 24) - 12;
        const uint8_t dir[3] = {(data.step.x > 0), (data.step.y > 0), data.side & 0xFF};

        auto tile = ILevelSystem->GetTileAtFast(data.mapPos.x, data.mapPos.y);
        ILevelSystem->AddBulletHole(tile, tex, dir, 2.f); // 3 works well
      //  engine->log("{%d %d}  textureY %f textureStep %f y_off %d perp %f lh %d ds %d", tex.x, tex.y, textureY, texStep, y_off, perpWallDist, lineHeight, drawStart);
        // engine->dbg("shot %i %i | %i", tile->m_vecPosition.x, tile->m_vecPosition.y, tile->m_nDecals);
    }

    return true;
}

void CBaseWeapon::Reload()
{


    auto curTick = IEngineTime->GetCurLoopTick();
    if(m_reserveammo == 0 ) return;
    if(m_nNextReload > curTick) return;

     this->OnReload(); 
     m_nNextReload = curTick + m_data.iReloadTime;
     if(m_reserveammo > m_data.iMaxAmmo)
     {
         m_clip = m_data.iMaxAmmo; 
        m_reserveammo -= m_clip;
     }
     else{
         m_clip = m_reserveammo;
         m_reserveammo = 0;
     }
    
}

void CBaseWeapon::OnSetOwnerEntity()
{

    m_pOwner = IEntitySystem->GetEntity<CBaseRenderable>(m_iOwnerID);
} // todo:s


