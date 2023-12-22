#pragma once
#include <common.hpp>
#include <entity/components/CBaseEntityComponent.hpp>
#include <types/CTime.hpp>





using behaviourFn = std::function<void(looptick_t, looptick_t)>;

struct behaviour_t 
{
    behaviourFn fn;
};

class CBehaviourControl : protected CBaseEntityComponent
{
public:
    CBehaviourControl(CBaseEntity* m_parent) : CBaseEntityComponent(m_parent, this, m_parent->GetID()), m_locked(false) {}
    virtual ~CBehaviourControl() {}

    virtual void OnCreate() { Debug(true); m_lastChange = 0; m_nextChange = 0; }
    virtual void OnUpdate();

    void AddBehaviour(const std::string& name, const behaviour_t& behave, bool is_default = false){
        auto add = behaviours.emplace(name, behave);
       if( !add.second ){
            Error("failed to add behaviour %s", name.c_str()); return;
       }
       if(is_default){
        m_default = add.first->first;
         dbg("added default behaviour %s", m_default.c_str());
         m_current = m_default;
       }
       else  dbg("added behaviour %s", name.c_str());
    }

    void ChangeBehaviour(const std::string& name); 

    void SetLockedBehaviour(const std::string& name){
        ChangeBehaviour(name);
        m_locked = true;
    }
    void Unlock() { m_locked = false; }
    void Default() { m_current = m_default; }

    auto GetCurrentBehaviour() const { return m_current; }
private:
    std::unordered_map<std::string, behaviour_t> behaviours;
    std::string m_current;
    std::string m_default;
    looptick_t m_lastChange;
    looptick_t m_nextChange;
    bool m_locked;
};