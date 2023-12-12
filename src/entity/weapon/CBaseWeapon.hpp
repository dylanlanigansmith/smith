#pragma once

#include <common.hpp>

#include <entity/CBaseRenderable.hpp>
#include <entity/components/animation/CAnimationController.hpp>
#include <entity/components/CBaseEntityComponent.hpp>
#include <logger/logger.hpp>

#include <entity/CBaseRenderable.hpp>
#include <util/misc.hpp>
#define SMITH_PLAYER_INV_SLOTS 6


//https://forum.zdoom.org/viewtopic.php?t=71476
//https://forum.zdoom.org/viewtopic.php?t=70605

//https://www.doomworld.com/forum/post/2310367 really fuckin sick 
//https://rekkimaru.itch.io/fps-gun-sprites

//https://ezgif.com/gif-to-sprite/ //amazing
struct weapon_data_t{
    double flRange;
    float flDamage;
    int iDamageMod;
    int iReloadTime;
    uint32_t iMaxAmmo;
    uint8_t nAmmoType;

    float GetDamage(){
        return Util::SemiRandRange(flDamage - iDamageMod / 1.5, flDamage + iDamageMod / 2);
    }
};

class CBaseWeapon : public CLogger //should include component and logging
{
public:
    CBaseWeapon(CBaseRenderable* m_pOwner, const std::string& m_szName) : CLogger(m_szName), m_pOwner(m_pOwner), m_szName(m_szName) {}
    virtual ~CBaseWeapon(){}
    virtual void OnCreate() = 0;
    virtual void Render(CRenderer* renderer) = 0;
    virtual void OnUpdate() = 0;
    virtual void Shoot();
    virtual void Reload();
    virtual void OnReload() = 0;
    virtual void OnShoot() = 0;
    virtual void SetOwnerEntity(hEntity m_iOwnerID) { this->m_iOwnerID = m_iOwnerID; OnSetOwnerEntity();}

    auto& GetName() const { return m_szName; }


    auto& GetData() const { return m_data; }
    auto GetCurrentAmmo() const { return m_clip; }
    auto GetReserveAmmo() const { return m_reserveammo; }
private: 

    void OnSetOwnerEntity();
protected:
    texture_t* m_texture;
    uint64_t m_nFireRate;
    uint64_t m_nNextShot = 0;

    uint64_t m_nNextReload = 0;
    uint32_t m_iShotsFired;
    weapon_data_t m_data;
    int m_clip;
    int m_reserveammo; 
    hEntity m_iOwnerID;
    CBaseRenderable* m_pOwner = nullptr;
    
    std::string m_szName;
};