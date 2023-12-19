#include "CWeaponSMG.hpp"
#include <entity/player/CPlayer.hpp>
#include <renderer/renderer.hpp>
#include <util/misc.hpp>
#include <engine/engine.hpp>

/*
need to figure out collisions for real

a top down 2d view in editor seems to recurringly be helpful as well

also do something fun because enemies are frustrating

*/


void CWeaponSMG::Render(CRenderer *renderer)
{
    static constexpr float flx = SCREEN_WIDTH / 17.0667f;
    static constexpr float fly = SCREEN_HEIGHT / -3.6f;
    static constexpr float gx = SCREEN_WIDTH / 5.9534f;
   
    anim_smg.Draw(renderer, {90.f, 5.f});
}

void CWeaponSMG::OnUpdate()
{
    anim_smg.OnUpdate();
 
    //need to add states & animation states
}

void CWeaponSMG::OnShoot()
{
   anim_smg.PlaySequenceByName("shoot0", true);
  
     engine->SoundSystem()->PlaySound("dev_gunshot0", 1.0); //needs a sound
}

void CWeaponSMG::OnReload()
{
    anim_smg.PlaySequenceByName("reload0");
}

void CWeaponSMG::OnCreate()
{
    // setup data
    this->m_nNextShot = 0;
    this->m_nFireRate = 3;
    this->m_data.flDamage = 8.0;
    this->m_data.iDamageMod = 6;
    this->m_data.iReloadTime = 80;
    this->m_clip = this->m_data.iMaxAmmo = 60;
    this->m_data.nAmmoType = 1;
    this->m_reserveammo =  this->m_data.iMaxAmmo * 2;

    anim_smg.AddDefaultSequenceByName("default0");
    anim_smg.AddSequenceByName("shoot0");
    anim_smg.AddSequenceByName("equip0");
    anim_smg.AddSequenceByName("reload0");

    

   
    if (m_pOwner == nullptr)
    {
        log("yo im a gun and im having some fuckin issues finding out who i belong to");
        return;
    }
    assert(m_pOwner->IsLocalPlayer());
}