#pragma once

#include <common.hpp>
#include <types/Vector.hpp>
#include <util/rtti.hpp>
#include <util/hash_fnv1a.hpp>


#define ENTREG_INC <interfaces/IEntitySystem/EntRegistry.hpp>

#define SET_ENT_NAME() this->m_szName = Util::getClassName(this)
#define SET_ENT_SUBNAME() this->m_szSubclass = Util::getClassName(this)
#define SET_ENT_TYPE() this->m_nType = Util::fnv1a::Hash64(this->m_szName.c_str())
#define ENT_SETUP() SET_ENT_NAME(); SET_ENT_TYPE(); 

//typedef uint32_t hEntity;


class hEntity
{
public:
    hEntity(uint32_t value = 0) : value(value) {}
    operator uint32_t() const { return value; }
    operator std::string() const { return std::to_string(value); }

private:
    uint32_t value;
};

class CBaseEntity
{
public:
    CBaseEntity() {}
    CBaseEntity(int m_iID) : m_iID(m_iID) { m_szName="CBaseEntity"; m_szSubclass=""; }
    virtual ~CBaseEntity(){}
    const auto GetID() { return m_iID; }
    const auto GetPosition() { return m_vecPosition; }
    virtual void SetPosition(double x, double y, double z = 0.0);
    virtual void SetPosition(const Vector& pos) { SetPosition(pos.x, pos.y, pos.z); }
    virtual const std::string GetName() {return m_szName;}
    virtual const std::string GetSubclass() {return m_szSubclass;}
    virtual const uint64_t GetType() { return m_nType; }
    virtual bool IsLocalPlayer() { return false;}
    virtual bool IsRenderable() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual float GetBounds() const { return 0.34f; }
    virtual void OnCollisionWith(CBaseEntity* hit) {}
    virtual void WhenCollidedBy(CBaseEntity* hitter) {}
    virtual bool IsBlocking() const { return true; }
    virtual bool IsShootable() const { return false; }
    virtual void OnHit(int damage, int position = 0) {}
protected:
    virtual void OnSetPosition(const Vector2& old_pos, const Vector2& new_pos) {}
protected:
    uint64_t m_nType; //fnv64 of name
    std::string m_szName;
    std::string m_szSubclass;
    Vector m_vecPosition;
    hEntity m_iID;
};

