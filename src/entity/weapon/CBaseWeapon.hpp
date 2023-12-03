#pragma once

#include <common.hpp>

#include <entity/CBaseRenderable.hpp>
#include <entity/components/animation/CAnimationController.hpp>
#include <entity/components/CBaseEntityComponent.hpp>
#include <logger/logger.hpp>

#include <entity/CBaseRenderable.hpp>




//https://forum.zdoom.org/viewtopic.php?t=71476
//https://forum.zdoom.org/viewtopic.php?t=70605

//https://www.doomworld.com/forum/post/2310367 really fuckin sick 
//https://rekkimaru.itch.io/fps-gun-sprites
struct weapon_data_t{
    double flRange;
    float flDamage;
    uint32_t iMaxAmmo;
    uint8_t nAmmoType;
};

class CBaseWeapon : public CLogger //should include component and logging
{
public:
    CBaseWeapon( const std::string& m_szName) : CLogger(m_szName), m_szName(m_szName) {}
    virtual ~CBaseWeapon(){}
    virtual void OnCreate() = 0;
    virtual void Render(CRenderer* renderer) = 0;
    virtual void OnUpdate() = 0;
    virtual void Shoot() = 0;
    virtual void SetOwnerEntity(hEntity m_iOwnerID) { this->m_iOwnerID = m_iOwnerID; OnSetOwnerEntity();}
    CAnimController* m_anim;
private: 

    void OnSetOwnerEntity();
protected:
    texture_t* m_texture;
    uint64_t m_nFireRate;
    uint64_t m_nNextShot = 0;
    uint32_t m_iShotsFired;
    weapon_data_t m_data;
    hEntity m_iOwnerID;
    CBaseRenderable* m_pOwner = nullptr;
    
    std::string m_szName;
};