#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <interfaces/CBaseInterface.hpp>

#include <entity/CBaseEntity.hpp>
#include <entity/player/CPlayer.hpp>








class CEntitySystem : public CBaseInterface
{
public:
    CEntitySystem() : CBaseInterface("IEntitySystem") {  m_iRenderableEntities = 0;}
    ~CEntitySystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override;
    virtual void OnLoopEnd() override;
    virtual void OnRenderStart() override;
    virtual void OnRenderEnd() override;
    virtual int NumRenderables() const { return m_iRenderableEntities; }

    virtual CPlayer* GetLocalPlayer();

    template <typename T> 
    T* GetEntity(hEntity h) //use shared ptr
    {
      if(h > entity_list.size() - 1)
        return nullptr;
    
       return static_cast<T*>(entity_list.at(h));
    }

    template <typename T> 
    T* AddEntity()
    {
        T* ent = new T(entity_list.size());
        auto base = (CBaseEntity*)(ent);
       
        entity_list.push_back(base);
         base->OnCreate();
        log("added entity %s | %lx | #%i", base->GetName().c_str(), base->GetType(), base->GetID());
        if(base->IsRenderable() && !base->IsLocalPlayer())
            m_iRenderableEntities++;
        return ent;
    }
    const auto& iterableList() { return entity_list; }
private:
    virtual void CreateLocalPlayer();
private:
    int m_iRenderableEntities;
    std::vector<CBaseEntity*> entity_list;
};
