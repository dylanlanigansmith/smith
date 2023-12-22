#pragma once

#include <common.hpp>
#include <types/Vector.hpp>
#include <util/rtti.hpp>
#include <util/hash_fnv1a.hpp>
#include <entity/entity_types.hpp>

#define ENTREG_INC <interfaces/IEntitySystem/EntRegistry.hpp>

#define SET_ENT_NAME() this->m_szName = Util::getClassName(this)
#define SET_ENT_SUBNAME() this->m_szSubclass = Util::getClassName(this)
#define SET_ENT_TYPE() this->m_nType = Util::fnv1a::Hash64(this->m_szName.c_str())
#define ENT_SETUP() SET_ENT_NAME(); SET_ENT_TYPE(); 


enum EntityTeams : uint8_t
{
    Team_None = 0,
    Team_Ally,
    Team_Enemy
};



class CBaseEntity
{
    friend class CEditor; //aw geez
public:
    CBaseEntity() {}
    CBaseEntity(int m_iID) : m_iID(m_iID), m_team(Team_None) { m_szName="CBaseEntity"; m_szSubclass=""; }
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
    virtual void OnHit(int damage, int position = 0) {}
    //Universal Bullet Hit Reg Goes Here!

    virtual float GetBounds() const { return 0.34f; }
    virtual void OnCollisionWith(CBaseEntity* hit) {}
    virtual void WhenCollidedBy(CBaseEntity* hitter) {}

    virtual bool IsStatic() const { return false; }
    virtual bool IsBlocking() const { return true; }
    virtual bool IsShootable() const { return false; }
    

    auto GetHealth() const { return m_health; }
    auto GetMaxHealth() const { return m_maxhealth; }
    auto IsAlive() const { return (m_health > 0); }
    virtual void SetHealth(int newhealth) { m_health = newhealth; }
    virtual bool TakesDamage() const { return false; }
    virtual void TakeDamage(int damage) { m_health -= damage; if(m_health <= 0) OnDeath(); }
    

    auto IsEnemy() const { return m_team == Team_Enemy; }
    auto IsAlly() const { return m_team == Team_Ally; }
    auto HasTeam() const { return m_team != Team_None; }
    auto  GetTeam() const { return m_team; }
    
    virtual bool IsSerializable() const { return false; }
protected:
    virtual void OnSetPosition(const Vector2& old_pos, const Vector2& new_pos) {}
    virtual void OnDeath() {}

protected:
    uint64_t m_nType; //fnv64 of name
    std::string m_szName;
    std::string m_szSubclass;
    Vector m_vecPosition;
    hEntity m_iID;

    int m_health;
    int m_maxhealth;
    uint8_t m_team;
};

