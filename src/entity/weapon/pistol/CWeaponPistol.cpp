#include "CWeaponPistol.hpp"
#include <entity/player/CPlayer.hpp>
#include <renderer/renderer.hpp>
#include <util/misc.hpp>
#include <engine/engine.hpp>

/*
need to figure out collisions for real

a top down 2d view in editor seems to recurringly be helpful as well

also do something fun because enemies are frustrating

*/


void CWeaponPistol::Render(CRenderer *renderer)
{
    static constexpr float flx = SCREEN_WIDTH / 11.5f;
    static constexpr float fly = SCREEN_HEIGHT / -4.6f; //360 p -3.6
    static constexpr float gx = SCREEN_WIDTH / 5.9534f;

     auto player = (CPlayer*)(m_pOwner);
    float bobY =  12.f + player->GetViewModel().GetBobY();
    anim_flash.Draw(renderer, {flx, fly + bobY}, 205);
    anim_pistol.Draw(renderer, {gx + player->GetViewModel().GetBobX(), bobY }); //10 + IEntitySystem->GetLocalPlayer()->GetPosition().z / 2 + IEntitySystem->GetLocalPlayer()->GetPosition().z / 3
}

void CWeaponPistol::OnUpdate()
{
    anim_pistol.OnUpdate();
    anim_flash.OnUpdate();

}

void CWeaponPistol::OnShoot()
{
   anim_pistol.PlaySequenceByName("shoot0");
    anim_flash.PlaySequenceByName("flash0");
     engine->SoundSystem()->PlaySound("dev_gunshot0", 1.0);
}

void CWeaponPistol::OnReload()
{
    anim_pistol.PlaySequenceByName("reload0");
}

void CWeaponPistol::OnCreate()
{
    // setup data
    this->m_nNextShot = 0;
    this->m_nFireRate = 12;
    this->m_data.flDamage = 13.0;
    this->m_data.iDamageMod = 7;
    this->m_data.iReloadTime = 28;
    this->m_clip = this->m_data.iMaxAmmo = 18;
    this->m_data.nAmmoType = 1;
    this->m_reserveammo =  this->m_data.iMaxAmmo * 2;

    anim_pistol.AddDefaultSequenceByName("default0");
    anim_pistol.AddSequenceByName("shoot0");
    anim_pistol.AddSequenceByName("reload0");

    anim_flash.AddDefaultSequenceByName("default0");
    anim_flash.AddSequenceByName("flash0");

   
    if (m_pOwner == nullptr)
    {
        log("yo im a gun and im having some fuckin issues finding out who i belong to");
        return;
    }
    assert(m_pOwner->IsLocalPlayer());
}
