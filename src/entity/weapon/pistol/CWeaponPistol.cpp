#include "CWeaponPistol.hpp"
#include <entity/player/CPlayer.hpp>
#include <renderer/renderer.hpp>

#include <engine/engine.hpp>

void CWeaponPistol::Render(CRenderer *renderer)
{
    m_flash->DrawFrame(renderer, {75, -200}, 190);
    m_anim->DrawFrame(renderer, {215, 0});
}

void CWeaponPistol::OnUpdate()
{
    m_anim->OnUpdate();
    m_flash->OnUpdate();
}

void CWeaponPistol::Shoot()
{
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem"); // real collisions just use 2d top down sdl rect
    static auto owner = static_cast<CPlayer *>(m_pOwner);
   auto curTick = IEngineTime->GetCurLoopTick();
    if(m_nNextShot > curTick) return;

    m_nNextShot = curTick + m_nFireRate;
    static constexpr auto hShoot = Anim::GetSequenceHandle("shoot0");
    m_anim->PlaySequence(hShoot);
    static constexpr auto hFlash = Anim::GetSequenceHandle("flash0");
    m_flash->PlaySequence(hFlash);
    auto cam = owner->Camera();
    auto pos = owner->GetPosition();

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
        // Check if ray has hit a wall o
        if (ILevelSystem->GetMapAt(mapPos.x, mapPos.y) > 0)
            hit = 1;
    }

    // FOR BULLET HOLES ^

    if (hit)
    {
        auto tile = ILevelSystem->GetTileAt(mapPos.x, mapPos.y);
        engine->log("shot %i %i", tile->m_vecPosition.x, tile->m_vecPosition.y);
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
        //ILevelSystem->AddBulletHole(tile, tex.x, radius)
    }
}
void CWeaponPistol::OnCreate()
{
    //setup data
    this->m_nNextShot = 0;
    this->m_nFireRate =  16;


    /*
    [ default 80x80] [ shoot1 80x80]  [ shoot2 80x80]
    [ reload0 80x96] [ reload1 80x96] [ reload2 80xa lot]
    [ reload3 80x96] [ reload4 80x96] [ reload5 80x96]

    287x285
    */
    m_anim = new CAnimController("pistol.png", {"pistol",{360,360}, {65, 176, 70, 255},{65, 176, 70, 255} },
    CAnimSequence("shoot0", 2, 
    std::vector<sequence_frame>{
        sequence_frame({0,0,80,80}, 0),  sequence_frame({80,0,80,80}, 1), 
         sequence_frame({160,0,80,80}, 2),  sequence_frame({80,0,80,80}, 3), 
          sequence_frame({0,0,80,80}, 4)
    })   
    );
    m_flash = new CAnimController("flash.png", {"flash",{256,256}, {0, 255, 255, 255}, {0, 0, 0, 255} },
        CAnimSequence("default", 1, 
    std::vector<sequence_frame>{
        sequence_frame({0,0,1,1}, 0),
    })   
    );
    m_flash->AddSequence(CAnimSequence("flash0", 1, 
    std::vector<sequence_frame>{
        sequence_frame({0,0,1,1}, 0), sequence_frame({0,0,1,1}, 1), sequence_frame({0,0,71,86}, 2),  sequence_frame({71,0,70,86}, 3), 
         sequence_frame({143,0,70,85}, 4), 
    }));
    if (m_pOwner == nullptr)
    {
        engine->log("yo im a gun and im having some fuckin issues finding out who i belong to");
        return;
    }
    assert(m_pOwner->IsLocalPlayer());
}
