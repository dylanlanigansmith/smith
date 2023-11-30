#include "ILevelSystem.hpp"
#include <engine/engine.hpp>
#include <entity/prop/objects/CBarrel.hpp>
#include <entity/prop/objects/CGreenLight.hpp>
#include <entity/prop/objects/CPillar.hpp>

#include <data/level.hpp>

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
    pillar->SetPosition(3, 12);
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
    if(type == 1)
        return tile->m_pTextureFloor;
    else 
        return tile->m_pTextureCeiling;

}

texture_t* CLevelSystem::GetTexturePlane(bool is_floor, int x, int y)
{
    x = std::clamp(x, 0, MAP_SIZE - 1); 
    y = std::clamp(y, 0, MAP_SIZE - 1); 
    if(!is_floor)
        return GetTextureAt(x,y, 2);
    
   //  bool checkerBoardPattern = (int(x + y)) & 1;
    return GetTextureAt(x,y, 1);
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