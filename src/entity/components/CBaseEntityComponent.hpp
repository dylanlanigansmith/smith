#pragma once
#include <common.hpp>
#include <logger/logger.hpp>
#include <entity/CBaseEntity.hpp>



class CBaseEntityComponent : public CLogger
{
public:
     template <typename T> 
    CBaseEntityComponent(CBaseEntity* m_parent, T* child, const std::string& m_szName) : CLogger(m_parent->GetName(), child, m_szName), 
                                                m_szName(m_szName), m_parent(m_parent) {}

      template <typename T> 
    CBaseEntityComponent( T* child, const std::string& m_szName) : CLogger( child, m_szName), 
                                                m_szName(m_szName), m_parent(0) {}

    virtual ~CBaseEntityComponent(){}
    virtual void OnCreate() = 0;
    virtual void OnUpdate() = 0;
protected:
    const std::string m_szName;
    CBaseEntity* m_parent;
};