#include "CBehaviour.hpp"
#include <engine/engine.hpp>

void CBehaviourControl::OnUpdate()
{
    if(m_current.empty() || m_default.empty()){
        return Error("we don't have any behaviours! current empty? %d default empty? %d",m_current.empty(),m_default.empty());
    }
    behaviour_t behave{};
    try
    {
      behave = behaviours.at(m_current);
    }
    catch(const std::exception& e)
    {
        Error("CBehaviourControl::OnUpdate(): failed w/ %s", e.what()); 
    }

    return behave.fn(m_lastChange, IEngineTime->GetCurLoopTick());
}

void CBehaviourControl::ChangeBehaviour(const std::string &name)
{
    if(m_locked) return;
    m_lastChange = IEngineTime->GetCurLoopTick();
    m_current = name;
}
