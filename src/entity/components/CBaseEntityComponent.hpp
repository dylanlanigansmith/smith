#pragma once
#include <common.hpp>
#include <logger/logger.hpp>
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

//slow redux 

class CBaseComponent : public CLogger
{
public:
     template <typename T> 
    CBaseComponent(CBaseEntity* m_parent, T* child, const std::string& m_szName) : CLogger(m_parent->GetName(), child, m_szName), 
                                                m_szName(m_szName), m_parent(m_parent) {}

      template <typename T> 
    CBaseComponent( T* child, const std::string& m_szName) : CLogger( child, m_szName), 
                                                m_szName(m_szName), m_parent(0) {}

    virtual ~CBaseComponent(){}
    virtual void OnCreate() = 0;
    virtual void OnUpdate() = 0;
protected:
    const std::string m_szName;
    CBaseEntity* m_parent;
};