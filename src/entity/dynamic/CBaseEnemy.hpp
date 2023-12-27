#pragma once
#include <logger/logger.hpp>
#include <util/misc.hpp>
#include <entity/CBaseRenderable.hpp>

#include <entity/dynamic/enemy_shared.hpp>
#include <entity/components/pathfinder/CPathFinder.hpp>
#include <entity/CMove.hpp>
#include "EntView.hpp"
#include "enemy_shared.hpp"

#include "CDynamicEntity.hpp"
class CBaseEnemy : virtual public CDynamicEntity
{
public:
    CBaseEnemy(int m_iID) : CDynamicEntity(m_iID), m_path(this) { m_team = Team_Enemy; }
    virtual ~CBaseEnemy() {}
    virtual void CreateRenderable() = 0;
    virtual void OnRenderStart() {}
    virtual void OnRenderEnd() {}
    virtual void Render(CRenderer* renderer) = 0;
    virtual void OnUpdate() = 0;
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void OnHit(int damage, const IVector2& position) = 0;

    virtual bool HitDetect(CCamera* camera, const IVector2 &point, IVector2 *textpos = nullptr) = 0;

    virtual bool HasLoot() const = 0;
    virtual int Loot() {if(!IsAlive()){ m_loot.LootTaken(); m_bounds = 0.0;
                    return Util::SemiRandRange(m_loot.m_amount.first, m_loot.m_amount.second);  } return -1;  }
    
    virtual void SetSubType(int type = 0) = 0;
    
    virtual bool Attack(CBaseEntity* target) = 0;
    virtual float GetDamageModForHit(const IVector2& pt) = 0;

    auto GetPathFinder() { return &m_path; }
    auto GetDestination() const { return m_headingTo; }
    auto& CombatStats() const { return m_combat; }


    static void SetIgnorePlayer(bool s) { m_ignoringPlayer = s; }
    static bool IsIgnoringPlayer() { return m_ignoringPlayer; }
    static void ToggleIgnorePlayer() { m_ignoringPlayer = !m_ignoringPlayer; } 

protected:
    virtual void OnDeath() = 0;
    virtual Color GetPixelAtPoint(CCamera* camera, const IVector2 &point, IVector2 *textpos = nullptr) = 0;

    bool isEntityVisible(CBaseEntity* ent, double fov);
    bool isPointInFOV(const Vector2 &pos, double FOVAngleDegrees);
    bool CastRayToPoint(const Vector2& pos, float bounds);
protected:
    loot_t m_loot;
    combat_stats m_combat;
    CPathFinder m_path;


    

    Vector2 m_headingTo;

    static bool m_ignoringPlayer;
};

