#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <interfaces/CBaseInterface.hpp>

#include <entity/CBaseEntity.hpp>
#include <entity/player/CPlayer.hpp>



#include <util/hash_fnv1a.hpp>




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
    CBaseEntity* GetEntity(hEntity h) //use shared ptr
    {
      if(h > entity_list.size() - 1)
        return nullptr;
    
       return (entity_list.at(h));
    }


    template <typename T> 
    T* AddEntity()
    {
        T* ent = new T(entity_list.size());
        auto base = (CBaseEntity*)(ent);
       
        entity_list.push_back(base);
        
        log("added entity %s | %lx | #%i", base->GetName().c_str(), base->GetType(), base->GetID());
         base->OnCreate();
        if(base->IsRenderable() && !base->IsLocalPlayer())
            m_iRenderableEntities++;
        return ent;
    }
    const auto& iterableList() { return entity_list; }

    static constexpr auto CreateType(const char* str) { return Util::fnv1a::Hash64(str); }
private:
    virtual void CreateLocalPlayer();
private:
    int m_iRenderableEntities;
    std::vector<CBaseEntity*> entity_list;
};
