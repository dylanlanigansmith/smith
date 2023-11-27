#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <interfaces/CBaseInterface.hpp>

#include <entity/CBaseEntity.hpp>
#include <entity/player/CPlayer.hpp>








class CEntitySystem : public CBaseInterface
{
public:
    CEntitySystem() : CBaseInterface("IEntitySystem") { }
    ~CEntitySystem() override;
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override;
    virtual void OnLoopEnd() override;
    virtual void OnRenderStart() override;
    virtual void OnRenderEnd() override;
    

    virtual CPlayer* GetLocalPlayer();

    template <typename T> 
    T* GetEntity(uint32_t h) //use shared ptr
    {
      if(h > entity_list.size() - 1)
        return nullptr;
    
       return static_cast<T*>(entity_list.at(h));
    }
private:
    template <typename T> 
    void AddEntity()
    {
        T* ent = new T(entity_list.size());
        auto base = (CBaseEntity*)(ent);
        base->OnCreate();
        entity_list.push_back(base);
    }
    virtual void CreateLocalPlayer();
private:
    std::vector<CBaseEntity*> entity_list;
};
