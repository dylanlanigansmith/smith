#include "IEntitySystem.hpp"
#include <engine/engine.hpp>


register_entity_t* CEntitySystem::ent_registry = nullptr;

CEntitySystem::~CEntitySystem()
{
    
}
void CEntitySystem::OnCreate()
{
   
}

void CEntitySystem::OnShutdown()
{
    
}

void CEntitySystem::OnLoopStart()
{
    event_mgr.ProcessQueue();
}

void CEntitySystem::OnLoopEnd()
{
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    if(!ILevelSystem->IsLevelLoaded()) return;

    auto& update = IEngineTime->GetUpdateTimer();
    static constexpr double timeStep = 1000/TICKS_PER_S;
    if(update.Elapsed().ms() <= timeStep)
        return;
    
    update.Reset(IEngineTime->GetCurTime());

    for(auto& ent : entity_list){
        if(ent == nullptr){
            Error("we have a nullptr in the entity list {%li}", entity_list.size()); break;
        }
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

void CEntitySystem::RemoveAllButPlayer()
{
    warn("clearing ent list, AllButPlayer");
    auto b4 = entity_list.size();
    size_t cleaned = 0;
   for(auto& ent : entity_list){
        if(!ent) continue;
        if(!ent->IsLocalPlayer()){
            ent->OnDestroy();
            
            delete ent;
            ent = nullptr;
            cleaned++;
        }
    }

    entity_list.erase(std::remove_if(entity_list.begin(), entity_list.end(), [](CBaseEntity* ent){ return ent == nullptr; } ), entity_list.end());

    warn("removed %li entities", b4 - entity_list.size() );
    if(b4 - entity_list.size() != cleaned){
        Error("failed to remove entities in some form or fashion. initial size %li, freed count %li, size change (-) %li,  new size %li", b4, cleaned, b4 - entity_list.size(), entity_list.size());
    }
    if(entity_list.size() != 1){
        Error("failed to remove all entities but localplayer. initial size %li, freed count %li new size %li", b4, cleaned, entity_list.size());
    }
}

CEntitySystem* CEntitySystem::_interface()
{
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    return IEntitySystem;
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
