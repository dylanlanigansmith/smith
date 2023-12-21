#pragma once

#include "../CBaseProp.hpp"
#include <nlohmann/json.hpp>
#include ENTREG_INC


enum CLevelPropFlags : uint64_t {
    PROP_BLOCKING = 1ULL << 0,
    PROP_STATIC = 1ULL << 1,
    PROP_MECHANIC = 1ULL << 2,
    PROP_POS_FLOOR = 1ULL << 3,
    PROP_POS_CEILING = 1ULL << 4,
    PROP_EMITSOUND = 1ULL << 5,
    PROP_ISGROUP = 1ULL << 6,
    PROP_TRANSPARENT = 1ULL << 7,
    PROP_HAS_TRIGGER = 1ULL << 8,
    PROP_TRIGGER_COLLIDE = 1ULL << 9,
    PROP_TRIGGER_ACTION = 1ULL << 10,
    PROP_TRIGGER_LISTEN = 1ULL << 11,
    PROP_FIRESEVENTS = 1ULL << 12,
    PROP_CANDESTRUCT = 1ULL << 13,
    PROP_DESTRUCTSTRONG = 1ULL << 14,
    PROP_DESTRUCTWEAK = 1ULL << 15,
    PROP_ISLOOT = 1ULL << 16,
    PROP_LOOT_AMMO = 1ULL << 17,
    PROP_LOOT_HEALTH = 1ULL << 18,
    PROP_LOOT_NEEDS_DESTRUCT = 1ULL << 19,
    PROP_BOUNDS_AABB = 1ULL << 20,
    PROP_VARY_SCALE = 1ULL << 21,
    CLevelPropFlags_SIZE = 22
};


class CLevelProp : public CBaseProp
{
    friend class CEditor;
public:
    CLevelProp(int m_iID) : CBaseProp(m_iID), m_loaded(false) {}
    virtual ~CLevelProp() {}
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void CreateRenderable();
    virtual void Render(CRenderer* renderer);
    virtual void OnHit(int damage, int position = 0) {}
    
    virtual void OnCollisionWith(CBaseEntity* hit) {}
    virtual void WhenCollidedBy(CBaseEntity* hitter) {}


    virtual bool IsStatic() const { return false; }
    virtual bool IsBlocking() const { return IsPropBlocking(); }
    virtual bool IsShootable() const { return PropCanDestruct(); }
    virtual bool TakesDamage() const { return PropCanDestruct(); }
    virtual bool IsSerializable() const { return true; }
    virtual float GetBounds() const { return m_bounds; }

    virtual bool HasTexture() { return m_loaded; }
    auto IsLoaded() const { return m_loaded; }

    bool FromJSON(const json& js);
    json ToJSON();
protected:
    virtual void OnDeath() {}
    virtual void OnSetPosition(const Vector2& old_pos, const Vector2& new_pos) {}

    virtual void SetupTexture();

    virtual void LoadDefaults();

    virtual void UpdateData();
public:

    //the names for these suck because chatgpt writes them (with much persuasion)
    //it is worth the time savings
    inline bool IsPropBlocking() const { return m_flags & PROP_BLOCKING; }
    inline bool IsStaticProp() const { return m_flags & PROP_STATIC; }
    inline bool IsMechanicProp() const { return m_flags & PROP_MECHANIC; }
    inline bool IsPosFloor() const { return m_flags & PROP_POS_FLOOR; }
    inline bool IsPosCeiling() const { return m_flags & PROP_POS_CEILING; }
    inline bool EmitsSound() const { return m_flags & PROP_EMITSOUND; }
    inline bool IsPropGroup() const { return m_flags & PROP_ISGROUP; }
    inline bool IsTransparent() const { return m_flags & PROP_TRANSPARENT; }
    inline bool HasTrigger() const { return m_flags & PROP_HAS_TRIGGER; }
    inline bool IsPropTriggerCollide() const { return m_flags & PROP_TRIGGER_COLLIDE; }
    inline bool IsPropTriggerAction() const { return m_flags & PROP_TRIGGER_ACTION; }
    inline bool IsPropTriggerListen() const { return m_flags & PROP_TRIGGER_LISTEN; }
    inline bool IsPropFiresEvents() const { return m_flags & PROP_FIRESEVENTS; }
    inline bool PropCanDestruct() const { return m_flags & PROP_CANDESTRUCT; }
    inline bool UsePropDestructStrong() const { return m_flags & PROP_DESTRUCTSTRONG; }
    inline bool UsePropDestructWeak() const { return m_flags & PROP_DESTRUCTWEAK; }
    inline bool IsLoot() const { return m_flags & PROP_ISLOOT; }
    inline bool IsLootAmmo() const { return m_flags & PROP_LOOT_AMMO; }
    inline bool IsLootHealth() const { return m_flags & PROP_LOOT_HEALTH; }
    inline bool LootNeedsDestruct() const { return m_flags & PROP_LOOT_NEEDS_DESTRUCT; }
    inline bool UseBoundsAABB() const { return m_flags & PROP_BOUNDS_AABB; }
    inline bool VaryScale() const { return m_flags & PROP_VARY_SCALE; }

    inline void SetPropBlocking(bool enabled) {
    m_flags = enabled ? (m_flags | PROP_BLOCKING) : (m_flags & ~PROP_BLOCKING);
    }
    inline void SetPropStatic(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_STATIC) : (m_flags & ~PROP_STATIC);
    }
    inline void SetPropMechanic(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_MECHANIC) : (m_flags & ~PROP_MECHANIC);
    }
    inline void SetPropPosFloor(bool enabled) {
    m_flags = enabled ? (m_flags | PROP_POS_FLOOR) : (m_flags & ~PROP_POS_FLOOR);
    }
    inline void SetPropPosCeiling(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_POS_CEILING) : (m_flags & ~PROP_POS_CEILING);
    }
    inline void SetPropEmitsound(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_EMITSOUND) : (m_flags & ~PROP_EMITSOUND);
    }
    inline void SetPropIsGroup(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_ISGROUP) : (m_flags & ~PROP_ISGROUP);
    }
    inline void SetPropTransparent(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_TRANSPARENT) : (m_flags & ~PROP_TRANSPARENT);
    }
    inline void SetPropHasTrigger(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_HAS_TRIGGER) : (m_flags & ~PROP_HAS_TRIGGER);
    }
    inline void SetPropTriggerCollide(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_TRIGGER_COLLIDE) : (m_flags & ~PROP_TRIGGER_COLLIDE);
    }
    inline void SetPropTriggerAction(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_TRIGGER_ACTION) : (m_flags & ~PROP_TRIGGER_ACTION);
    }
    inline void SetPropTriggerListen(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_TRIGGER_LISTEN) : (m_flags & ~PROP_TRIGGER_LISTEN);
    }
    inline void SetPropFiresevents(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_FIRESEVENTS) : (m_flags & ~PROP_FIRESEVENTS);
    }
    inline void SetPropCandestruct(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_CANDESTRUCT) : (m_flags & ~PROP_CANDESTRUCT);
    }
    inline void SetPropDestructstrong(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_DESTRUCTSTRONG) : (m_flags & ~PROP_DESTRUCTSTRONG);
    }
    inline void SetPropDestructweak(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_DESTRUCTWEAK) : (m_flags & ~PROP_DESTRUCTWEAK);
    }
    inline void SetPropIsLoot(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_ISLOOT) : (m_flags & ~PROP_ISLOOT);
    }
    inline void SetPropLootAmmo(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_LOOT_AMMO) : (m_flags & ~PROP_LOOT_AMMO);
    }
    inline void SetPropLootHealth(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_LOOT_HEALTH) : (m_flags & ~PROP_LOOT_HEALTH);
    }
    inline void SetPropLootNeedsDestruct(bool enabled) {
    m_flags = enabled ? (m_flags | PROP_LOOT_NEEDS_DESTRUCT) : (m_flags & ~PROP_LOOT_NEEDS_DESTRUCT);
    }
    inline void SetPropBoundsAabb(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_BOUNDS_AABB) : (m_flags & ~PROP_BOUNDS_AABB);
    }
    inline void SetPropVaryScale(bool enabled) {
        m_flags = enabled ? (m_flags | PROP_VARY_SCALE) : (m_flags & ~PROP_VARY_SCALE);
    }
private:
    bool m_loaded;
    int64_t m_flags;
    std::string m_uid; 

    bool m_inherits;
    std::string m_parentname;
    std::string m_textureName;
    std::string m_altTextureName;
    Color m_color;
    sprite_draw_params draw_params;
    

    float m_bounds;
    BBoxAABB m_bbox;


    


    std::string m_specialname;
    std::string m_triggername; //for lookup in a database of functors for actions etc
    std::string m_callbackname;
    std::string m_actionname; 
    std::string m_slavename; //if controlling
    REGISTER_DEC_ENT(CLevelProp);
};