#include "ILevelSystem.hpp"
#include <engine/engine.hpp>
#include <entity/prop/objects/CBarrel.hpp>
#include <entity/prop/objects/CGreenLight.hpp>
#include <entity/prop/objects/CPillar.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>
#include <data/level.hpp>
#include <util/misc.hpp>
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

    LoadAndFindTexturesForMap();
    auto barrel = IEntitySystem->AddEntity<CBarrel>();
    barrel->SetPosition(12, 22);
    auto light = IEntitySystem->AddEntity<CGreenLight>();
    light->SetPosition(10, 12);
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

IVector2 CLevelSystem::FindEmptySpace() //Random
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