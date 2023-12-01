#pragma once

#include <common.hpp>
#include <types/Vector.hpp>
#include <util/rtti.hpp>
#include <util/hash_fnv1a.hpp>



#define SET_ENT_NAME() this->m_szName = Util::getClassName(this)
#define SET_ENT_SUBNAME() this->m_szSubclass = Util::getClassName(this)
#define SET_ENT_TYPE() this->m_nType = Util::fnv1a::Hash64(this->m_szName.c_str())
#define ENT_SETUP() SET_ENT_NAME(); SET_ENT_TYPE();

typedef uint32_t hEntity;

class CBaseEntity
{
public:
    CBaseEntity() {}
    CBaseEntity(int m_iID) : m_iID(m_iID) { m_szName="CBaseEntity"; m_szSubclass="";}
    virtual ~CBaseEntity(){}
    const auto GetID() { return m_iID; }
    const auto GetPosition() { return m_vecPosition; }
    virtual void SetPosition(double x, double y, double z = 0.0){ m_vecPosition = { x,y,z}; }
    virtual const std::string GetName() {return m_szName;}
    virtual const std::string GetSubclass() {return m_szSubclass;}
    virtual const uint64_t GetType() { return m_nType; }
    virtual bool IsLocalPlayer() { return false;}
    virtual bool IsRenderable() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;
   
protected:
    uint64_t m_nType; 
    std::string m_szName;
    std::string m_szSubclass;
    Vector m_vecPosition;
    hEntity m_iID;
};