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
    static constexpr float gx = SCREEN_WIDTH * 0.140625f;
    static constexpr float gy = SCREEN_HEIGHT * 0.0139f;

    auto player = (CPlayer*)(m_pOwner);
    anim_smg.Draw(renderer, {gx + player->GetViewModel().GetBobX(), gy + 10.f + player->GetViewModel().GetBobY()}); //640x360 = 90, 5   - IEntitySystem->GetLocalPlayer()->GetPosition().z
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

    anim_smg.AddDefaultSequenceByName("default0", {SCREEN_WIDTH * 0.703125f, SCREEN_HEIGHT *  0.65f}); //640x360 = 450x234
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
