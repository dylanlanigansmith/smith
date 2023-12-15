#include "CBaseWeapon.hpp"
#include <engine/engine.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>
#include <entity/dynamic/enemy/CEnemySoldier.hpp>
inline bool HitDetectPixelPerfect(CPlayer *player, CEnemySoldier *ent, IVector2 *textpos)
{
    auto crosshair_color = ent->GetPixelAtPoint(player->m_pCamera(), {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, textpos);
    //log("%x", crosshair_color);
    return (crosshair_color != 0u);
}


bool CBaseWeapon::Shoot()
{
     static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem"); // real collisions just use 2d top down sdl rect
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    static auto owner = static_cast<CPlayer *>(m_pOwner);
    auto curTick = IEngineTime->GetCurLoopTick();

    static looptick_t click = 0;
    if (m_nNextShot > curTick || m_clip == 0 || m_nNextReload > curTick){
        if(m_clip == 0 && click < curTick){
            click = curTick + TICKS_PER_S / 2; engine->SoundSystem()->PlaySound("empty-gun", 1.0);
        }
            
        return false;
    }
        

    m_nNextShot = curTick + m_nFireRate;
    m_clip --;
    
    this->OnShoot();
    

    auto cam = owner->Camera();
    auto pos = owner->GetPosition();

    const std::string enemyname = "CEnemySoldier";
    //collision detection
    
    auto tile = ILevelSystem->GetTileAt(IVector2::Rounded(pos.x, pos.y)); //this should be a function
    CEnemySoldier *hit_ent = nullptr;
    if (!tile->m_occupants.empty()) //player doesnt get tiled
    {
        static constexpr auto enemy_type = Util::fnv1a::Hash64("CEnemySoldier");
        for (auto &id : tile->m_occupants)
        {
            auto ent = IEntitySystem->GetEntity(id);
            if (ent == nullptr)
                continue;
            if (ent->IsLocalPlayer())
                continue;
            if (ent->GetType() == enemy_type)
            {
                hit_ent = (CEnemySoldier *)ent;
                IVector2 textpos;
                if( HitDetectPixelPerfect(owner, hit_ent, &textpos)){ //should return position
                    dbg("hit");
                    int pos = Util::SemiRandRange(0, 8) * -1;
                    hit_ent->OnHit(Util::SemiRandRange(8, 16), pos);

                    return true;
                }
            }
        }
    }
    int screenx = (SCREEN_WIDTH / 2);
    double camOffset = 2.0 * screenx / (double)SCREEN_WIDTH - 1.0;
    // dda dda dda
    Vector2 rayDir = {
        cam.m_vecDir.x + cam.m_vecPlane.x * camOffset,
        cam.m_vecDir.y + cam.m_vecPlane.y * camOffset};

    // which box of the map we're in
    IVector2 mapPos(pos.x, pos.y);

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
        sideDist.x = (pos.x - mapPos.x) * deltaDist.x;
    }
    else
    {
        step.x = 1;
        sideDist.x = (mapPos.x + 1.0 - pos.x) * deltaDist.x;
    }
    if (rayDir.y < 0)
    {
        step.y = -1;
        sideDist.y = (pos.y - mapPos.y) * (deltaDist.y);
    }
    else
    {
        step.y = 1;
        sideDist.y = (mapPos.y + 1.0 - pos.y) * (deltaDist.y);
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
        /*

        so we really want to check if an entity is in the square along the way
        if so then we do a more precise cast?

        */
        auto tile = ILevelSystem->GetTileAt(mapPos);
        CEnemySoldier *hit_ent = nullptr;
        if (!tile->m_occupants.empty())
        {
            static constexpr auto enemy_type = Util::fnv1a::Hash64("CEnemySoldier");
            for (auto &id : tile->m_occupants)
            {
                auto ent = IEntitySystem->GetEntity(id);
                if (ent == nullptr)
                    continue;
                if (ent->IsLocalPlayer())
                    continue;
                if (ent->GetType() == enemy_type)
                {
                    hit_ent = (CEnemySoldier *)ent;
                   IVector2 textpos;
                    if( HitDetectPixelPerfect(owner, hit_ent, &textpos)){ //should return position
                       
                        int pos = Util::SemiRandRange(0, 8) * -1;
                        hit_ent->OnHit(Util::SemiRandRange(m_data.flDamage - m_data.iDamageMod / 1.5, m_data.flDamage + m_data.iDamageMod / 2), pos); //soooo the animation should play on the texture not rendered on top.. new CTextureAnimationController time

                        return true;
                    }
                }
            }
        }
        // Check if ray has hit a wall o
        if (tile->m_nType == Level::Tile_Wall )
            hit = 1;
    }
    // FOR BULLET HOLES ^

    if (hit)
    {
        auto tile = ILevelSystem->GetTileAtFast(mapPos.x, mapPos.y);
        // engine->dbg("shot %i %i | %i", tile->m_vecPosition.x, tile->m_vecPosition.y, tile->m_nDecals);
        if (side == 0)
            perpWallDist = (sideDist.x - deltaDist.x);
        else
            perpWallDist = (sideDist.y - deltaDist.y);
        double wallX; // where exactly the wall was hit
        if (side == 0)
            wallX = pos.y + perpWallDist * rayDir.y;
        else
            wallX = pos.x + perpWallDist * rayDir.x;
        wallX -= floor((wallX));
        IVector2 tex;
        int textW = 64;
        tex.x = int(wallX * double(textW));
        if (side == 0 && rayDir.x > 0)
            tex.x = textW - tex.x - 1;
        if (side == 1 && rayDir.y < 0)
            tex.x = textW - tex.x - 1;

        tex.y = 32 + Util::SemiRandRange(0, 16) - 8;

        const uint8_t dir[3] = {(step.x > 0), (step.y > 0), side & 0xFF};
        ILevelSystem->AddBulletHole(tile, tex, dir, 2.f); // 3 works well
    }

    return true;
}

void CBaseWeapon::Reload()
{
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");

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
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    m_pOwner = IEntitySystem->GetEntity<CBaseRenderable>(m_iOwnerID);
} // todo:s


