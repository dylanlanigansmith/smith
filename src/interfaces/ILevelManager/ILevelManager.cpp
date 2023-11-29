#include "ILevelManager.hpp"
#include <engine/engine.hpp>
#include <entity/prop/objects/CBarrel.hpp>
#include <entity/prop/objects/CGreenLight.hpp>
#include <entity/prop/objects/CPillar.hpp>
void CLevelSystem::OnCreate()
{
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
    AddMapTexture(100, "colorstone.png"); //ceiling
    AddMapTexture(101, "bluestone.png"); //check1
    AddMapTexture(102, "purplestone.png"); //check2

    AddMapTexture(1, "wood.png");
    AddMapTexture(2, "redbrick.png");
    AddMapTexture(3, "greystone.png");
    AddMapTexture(4, "mossy.png");
    AddMapTexture(5, "pillar.png");


}

void CLevelSystem::AddMapTexture(int id, const std::string& name)
{
    auto hTex = m_TextureSystem->LoadTexture(name);
    if(!m_TextureSystem->IsHandleValid(hTex)){
        log("failed to add map texture for mapid %i", id); return;
    }
    auto succ = level_textures.emplace(id, hTex);
    if(!succ.second){
         log("failed to add map texture to db for mapid %i handle %i", id, hTex); return;
    }
}





hTexture CLevelSystem::GetTextureAt(int x, int y)
{
    int id = GetMapAt(x,y);
    return FindTextureForMapObject(id);

}

hTexture CLevelSystem::GetTexturePlane(bool is_floor, int x, int y)
{
    if(!is_floor)
        return FindTextureForPlane(false);
    
     bool checkerBoardPattern = (int(x + y)) & 1;
    return FindTextureForPlane(is_floor, checkerBoardPattern); 
}




hTexture CLevelSystem::FindTextureForMapObject(int obj)
{
    hTexture ret;
    try{
        ret =  level_textures.at(obj);
    }
    catch (const std::out_of_range& oor) {
        Error("unset texture for map obj %i", obj);
        
        ret = m_TextureSystem->ErrorTexture();
    }
   
   return ret;
}

hTexture CLevelSystem::FindTextureForPlane(bool is_floor, bool alt)
{
    if(!is_floor)
        return FindTextureForMapObject(100);
    if(alt)
        return FindTextureForMapObject(102);
    return FindTextureForMapObject(101);
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