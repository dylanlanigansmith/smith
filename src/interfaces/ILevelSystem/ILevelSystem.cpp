#include "ILevelSystem.hpp"
#include <engine/engine.hpp>
#include <entity/prop/objects/CBarrel.hpp>
#include <entity/prop/objects/CGreenLight.hpp>
#include <entity/prop/objects/CPillar.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>
#include <data/level.hpp>
#include <util/misc.hpp>
#include <renderer/render_helpers.hpp>
/*
For Editor:

Level Stuff:
We need to serialize the level class
Need to switch functions to new format

**Texture stuff:
**need to create texture database json thing [DONE]
**functions for finding textures [DONE]
final thing is clean up implementation once level stuff done

Tile stuff:
tiles should have decal support -> dont worry about it
lighting
-> make lightmap in editor

ImGui stuff:
cool way to render each tile (imgui selectable?)
ensure we can load textures for imgui as well
new system for it

Engine stuff:
Global states 
ie. in editor, 
add a menu 


Add editor, then add more entities
then add AI
then add a gun 

then network it
then add sound
then it is done


*/


void CLevelSystem::OnCreate()
{
   
}

void CLevelSystem::OnEngineInitFinish()
{
    static auto IResourceSystem = engine->CreateInterface<CResourceSystem>("IResourceSystem");
    if(!IResourceSystem->LoadLevel("lvldev"))
        return;
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    m_TextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem");

    static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
     constexpr Color sun = Color(255, 165, 0, 170);
    ILightingSystem->AddLight<CLightOverhead>({8.f, 8.4f, 1.f}, Color::CandleLight(), 0.5, 0.5f, 4.0);
    ILightingSystem->AddLight<CLightOverhead>({19.5f, 21.5f, 1.9f}, sun, 0.7, 0.7, 16.0);
    ILightingSystem->AddLight<CLightOverhead>({21.5f, 6.5f, 1.9f}, sun, 0.7, 0.7, 16.0);
    ILightingSystem->AddLight<CLightOverhead>({18.5f, 7.5f, 1.9f}, sun, 0.7, 0.7, 16.0);
   
     ILightingSystem->AddLight<CLightOverhead>({11.6f, 9.3f, 1.9f}, Color::FluorescentLight(), 0.6, 0.6, 4.0);
     ILightingSystem->AddLight<CLightOverhead>({6.3f, 12.3f, 1.9f}, Color::FluorescentLight(), 0.6, 0.6, 4.0);

     ILightingSystem->AddLight<CLightOverhead>({8.6f, 22.2f, 1.9f}, Color::CoolBlueLight(), 0.6, 0.6, 4.5);
      ILightingSystem->AddLight<CLightOverhead>({15.6f, 22.2f, 1.9f}, Color::DimRedLight(), 0.6, 0.6, 6.0);
    LoadAndFindTexturesForMap();
    auto barrel = IEntitySystem->AddEntity<CBarrel>();
    barrel->SetPosition(15.7, 9.5);
    auto light = IEntitySystem->AddEntity<CGreenLight>();
    light->SetPosition(19.5, 18.5);
     auto pillar = IEntitySystem->AddEntity<CPillar>();
    pillar->SetPosition(2, 12);

    auto enemy = IEntitySystem->AddEntity<CBaseEnemy>();
    enemy->SetPosition(9, 19);
   // enemy->Freeze(true);
   auto enemy2 = IEntitySystem->AddEntity<CBaseEnemy>();
   enemy2->SetPosition(12, 22);
    // enemy2->Freeze(true);
    auto enemy3 = IEntitySystem->AddEntity<CBaseEnemy>();
    enemy3->SetPosition(8, 2);
    // enemy3->Freeze(true);
}

tile_t *CLevelSystem::GetTileNeighbor(tile_t *tile, int dir) //nullptr if none
{
    IVector2 coords = tile->m_vecPosition;
    switch(dir)
    {
        case NORTH:
            coords.y -= 1; break;
        case EAST:
            coords.x += 1;break;
        case SOUTH:
            coords.y += 1;break;
        case WEST:
           coords.x -= 1;break;
        default:
            return nullptr;
    }

    auto neighbor = GetTileAt(coords);
    return neighbor;
}

IVector2 CLevelSystem::FindEmptySpace() // Random
{
    int bail = 0;
    while(1)
    {
        if(bail > 100 ){
            Error("findemptyspace bailed after %i tries- returning {1,1}", bail);
            return { 1,1 }; break;
        }

        int x = Util::SemiRandRange(0, MAP_SIZE - 1);
        int y = Util::SemiRandRange(0, MAP_SIZE - 1);
        bail++;
        if(GetMapAt(x,y) != Level::Tile_Empty)
            continue;
        return {x,y};
    }
}

bool CLevelSystem::IsCollision(const Vector& origin, const Vector& goal)
{
    //to raycast everything we would have to either dda or deduce direction and get face coords, or check all 4 lol

    Vector delta = goal - origin;
    auto tile = GetTileAt({goal.x, goal.y});
    auto tile2 = GetTileAt(Vector2(goal - delta * 0.1));
    if(!tile || !tile2) return true;
    auto type = tile->m_nType;
    auto type2 = tile2->m_nType;
    if(type == Level::Tile_Empty && type == type2) return false;
    if(type == Level::Tile_Wall) return true;

    //we have a fancy wall aw shit
    Ray_t ray = {
        .origin = origin,
        .direction = (goal - origin).Normalize()
    };

    auto wall = Render::GetLineForWallType(tile->m_vecPosition, type);
    Vector2 intersect;
    const double wall_thick = 0.1;
                BBoxAABB thickness = {
                .min = wall.p0,
                .max = wall.p1
                };
    if(!Util::RayIntersectsLineSegment(ray, wall, intersect))
        return false;
   // if(!Util::RayIntersectsBox(ray, thickness))
    //    return false;
    if( (Vector2(goal) - intersect).Length() > 0.1)
        return false;
    return true;

}

void CLevelSystem::LoadAndFindTexturesForMap()
{
    auto& world = m_Level->world;
    for(auto& row : world)
    {
        for(auto& tile : row){
           tile.m_pTexture = m_TextureSystem->GetTextureData(tile.m_hTexture);
           tile.m_pTextureCeiling = m_TextureSystem->GetTextureData(tile.m_hTextureCeiling);
           tile.m_pTextureFloor = m_TextureSystem->GetTextureData(tile.m_hTextureFloor);
        }
    }

//shit i forgot about ceilings
/*
    AddMapTexture(100, "colorstone.png"); //ceiling
    AddMapTexture(101, "bluestone.png"); //check1
    AddMapTexture(102, "purplestone.png"); //check2

    AddMapTexture(1, "wood.png");
    AddMapTexture(2, "redbrick.png");
    AddMapTexture(3, "greystone.png");
    AddMapTexture(4, "mossy.png");
    AddMapTexture(5, "pillar.png");
    AddMapTexture(69, "greystone.png");*/

}

void CLevelSystem::AddMapTexture(int id, const std::string& name)
{
    auto hTex = m_TextureSystem->FindTexture(name);
    if(!m_TextureSystem->IsHandleValid(hTex)){
        log("failed to add map texture for mapid %i", id); return;
    }
    auto succ = level_textures.emplace(id, hTex);
    if(!succ.second){
         log("failed to add map texture to db for mapid %i handle %i", id, hTex); return;
    }
}





texture_t* CLevelSystem::GetTextureAt(int x, int y,  uint8_t type) 
{
    auto tile = GetTileAt(x,y);
    if(!type)
        return tile->m_pTexture;
    if(tile->m_nType != Level::Tile_Empty && tile->m_nType == Level::Tile_Wall)
        return tile->m_pTexture;
    
    if(type == TileTexture_Ceiling)
        return tile->m_pTextureCeiling;
    else 
        return tile->m_pTextureFloor;

}

texture_t* CLevelSystem::GetTexturePlane(bool is_floor, int x, int y)
{
    x = std::clamp(x, 0, MAP_SIZE - 1); 
    y = std::clamp(y, 0, MAP_SIZE - 1); 
    auto tile = GetTileAt(x,y);
    if(tile->IsThinWall())
        return (is_floor) ? GetTextureAt(x,y, TileTexture_Floor) : GetTextureAt(x,y, TileTexture_Ceiling);
    if(!is_floor)
        return GetTextureAt(x,y, TileTexture_Ceiling);
    
   //  bool checkerBoardPattern = (int(x + y)) & 1;
    return GetTextureAt(x,y, TileTexture_Floor);
}

void CLevelSystem::AddBulletHole(tile_t* tile, const IVector2 pos, const uint8_t* side, float radius)
{
    #define MAX_DECALS 4

    if(tile->m_nDecals > MAX_DECALS){
        int index = (tile->m_nDecals - 1) % MAX_DECALS;
        auto pDecals = tile->m_pDecals;
        for (int i = 0; i < index; ++i) {
            if(pDecals == nullptr) break;
            pDecals = pDecals->m_pNextDecal;
        }
        if(pDecals == nullptr) return; //always nullptr!!

        pDecals->radius = radius;
       // pDecals->m_pNextDecal = nullptr;
        pDecals->texturePosition = pos;
        pDecals->side = side[2];
        pDecals->dir[0] = side[0];
        pDecals->dir[1] = side[1];
         tile->m_nDecals++;
        return;
    }

    auto hole = new decal_t;
    hole->radius = radius;
    hole->m_pNextDecal = nullptr;
    hole->texturePosition = pos;
    hole->side = side[2];
    hole->dir[0] = side[0];
    hole->dir[1] = side[1];

    if(tile->m_nDecals <= 0){
           tile->m_nDecals = 1;
           tile->m_pDecals = hole;
           return;
    }
    decal_t* pDecals = tile->m_pDecals;
    tile->m_nDecals++;
    int d = 0;
    while (pDecals != nullptr && pDecals->m_pNextDecal != nullptr)
    {
        
        auto c_pDecals = pDecals->m_pNextDecal;

        if( c_pDecals->m_pNextDecal == nullptr) break;
        pDecals = pDecals->m_pNextDecal;
        d++;
    }
    if(pDecals == nullptr || pDecals && pDecals->m_pNextDecal){
        delete hole; return;
    }

   // log("%i %i", tile->m_nDecals, d);
    pDecals->m_pNextDecal = hole;
}




hTexture CLevelSystem::FindTextureForMapObject(int obj)
{
    hTexture ret;
    try{
        ret =  level_textures.at(obj);
    }
    catch (const std::out_of_range& oor) {
        Error("unset texture for map obj %i", obj);
        
        ret = m_TextureSystem->ErrorTextureHandle();
    }
   
   return ret;
}



void CLevelSystem::OnShutdown()
{
}

void CLevelSystem::OnLoopStart()
{
}

void CLevelSystem::OnLoopEnd()
{
}

void CLevelSystem::OnRenderStart()
{
}

void CLevelSystem::OnRenderEnd()
{
}