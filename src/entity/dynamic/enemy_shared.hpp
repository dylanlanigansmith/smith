#pragma once
#include <entity/CBaseRenderable.hpp>
#include <util/misc.hpp>
struct combat_stats
{
    int m_damage_primary = 10;
    int m_damage_variable = 5; 
    int m_damage_alt = 5;
    float m_damage_mod = 1.0;
    float m_accuracy = 1.0;
    inline auto CalculateDamage() const {
        return m_damage_primary + (Util::SemiRandRange(0, m_damage_variable) - (m_damage_variable / 2) );
    }
    inline auto GetAccuracy() const { return m_accuracy;}
    inline auto GetDamage() const { return m_damage_primary;}
    inline auto GetAltDamage() const { return m_damage_alt;}
    inline auto GetDamageModifer() const { return m_damage_mod;}
};

struct loot_t
{
    int m_type{};
    std::pair<int, int> m_amount = {10,20};
    bool m_used = false;

    inline auto HasBeenLooted() const { return m_used; }
    inline void LootTaken() { m_used = true; }
};