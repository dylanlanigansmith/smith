#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <interfaces/CBaseInterface.hpp>

#include <entity/CBaseEntity.hpp>
#include <entity/player/CPlayer.hpp>



#include <util/hash_fnv1a.hpp>


typedef std::map<std::string, CBaseEntity*(*)()> register_entity_t; 
typedef std::pair<const std::string, CBaseEntity*(*)()> registered_ent_t;


class CEntitySystem : public CBaseInterface
{
    friend class CEditor;
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

    virtual void OnResourceLoadEnd() {  CreateLocalPlayer();}
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

    void RemoveAllButPlayer();
    const auto& iterableList() { return entity_list; }

    static constexpr auto CreateType(const char* str) { return Util::fnv1a::Hash64(str); }

    static auto GetRegistry(){
        if(!ent_registry){
            ent_registry =  new register_entity_t();
        }
        return ent_registry;
    }

    template <typename T>
    static CBaseEntity* AddEntByRegistry()
    {
        return _interface()->AddEntity<T>();
    }
   
private:
    static CEntitySystem* _interface();
    virtual void CreateLocalPlayer();
private:
    int m_iRenderableEntities;
    std::vector<CBaseEntity*> entity_list;

    static register_entity_t* ent_registry;
};


