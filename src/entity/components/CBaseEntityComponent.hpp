#pragma once
#include <common.hpp>

#include <entity/CBaseEntity.hpp>

class CBaseEntityComponent
{
public: 
    CBaseEntityComponent(CBaseEntity* m_pParent) : m_pParent(m_pParent) {m_iParentID = m_pParent->GetID(); }
    virtual void OnCreate() = 0;
    virtual void OnUpdate() = 0;    

    

private:


protected:
    hEntity m_iParentID;
    CBaseEntity* m_pParent = nullptr;
};