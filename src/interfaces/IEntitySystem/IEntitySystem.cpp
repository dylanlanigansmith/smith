#include "IEntitySystem.hpp"
#include <engine/engine.hpp>

CEntitySystem::~CEntitySystem()
{
    
}
void CEntitySystem::OnCreate()
{
    CreateLocalPlayer();
}

void CEntitySystem::OnShutdown()
{
    
}

void CEntitySystem::OnLoopStart()
{
    
}

void CEntitySystem::OnLoopEnd()
{
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    auto& update = IEngineTime->GetUpdateTimer();
    static constexpr double timeStep = 1000/TICKS_PER_S;
    if(update.Elapsed().ms() <= timeStep)
        return;
    
    update.Reset(IEngineTime->GetCurTime());

    for(auto& ent : entity_list){
        ent->OnUpdate();
    }
    IEngineTime -> m_loopticks++;
}

void CEntitySystem::OnRenderStart()
{
       for(auto& ent : entity_list){
        if(ent->IsRenderable()){
            auto renderable = static_cast<CBaseRenderable*>(ent);
            renderable->OnRenderStart();
        }
    }
}


void CEntitySystem::OnRenderEnd()
{
    for(auto& ent : entity_list){
        if(ent->IsRenderable()){
            auto renderable = static_cast<CBaseRenderable*>(ent);
            renderable->OnRenderEnd();
        }
    }
}

void CEntitySystem::CreateLocalPlayer()
{
    AddEntity<CPlayer>();
    log("created localplayer");
}

CPlayer* CEntitySystem::GetLocalPlayer()
{
    return GetEntity<CPlayer>((hEntity)0);
}
