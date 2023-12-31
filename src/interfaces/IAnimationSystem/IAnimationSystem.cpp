#include "IAnimationSystem.hpp"
#include <engine/engine.hpp>

CAnimationSystem::~CAnimationSystem()
{
}

void CAnimationSystem::OnLoopStart()
{
    
}

void CAnimationSystem::OnLoopEnd()
{
    
}

CAnimData* CAnimationSystem::GetSequence(const std::string& parent_name, const std::string& seq_name)
{
    std::string db_name = parent_name + std::string("_") + seq_name;
    return animation_list.at(db_name);
}

std::unique_ptr<CAnimSurface> CAnimationSystem::GetSurface(const IVector2 &size)
{
    return std::make_unique<CAnimSurface>(size);
}

nlohmann::json CAnimationSystem::ListToJson()
{
    json ret = json::object();

    for(auto& anim : animation_list)
    {
        ret.emplace(anim.first, anim.second->ToJson());
    }

    return ret;
}

void CAnimationSystem::ListFromJson(const nlohmann::json& js)
{

    for(auto& key : js.items()){
        auto add = new CAnimData();
        auto name = std::string(key.key()) ;
        add->FromJson(key.value());
        add->m_pTexture = ITextureSystem->FindOrCreatetexture(add->m_szTextureName);
        animation_list.emplace(name, add);
        dbg("added %s [%li frames] to list [%li]", name.c_str(), add->m_frames.size(), animation_list.size());

       // dbg("mask color %s 2 %s", add->GetMaskColor().s().c_str(), add->GetMaskColorAlt().s().c_str());
    }
    note("loaded %li animations", animation_list.size());
}

void CAnimationSystem::LoadSequences()
{



    
}


void CAnimationSystem::OnCreate()
{
    Debug(false);

    IResourceSystem->LoadAnimations();
}

void CAnimationSystem::OnShutdown()
{
    
}