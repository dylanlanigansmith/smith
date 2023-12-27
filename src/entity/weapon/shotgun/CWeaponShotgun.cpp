#include "CWeaponShotgun.hpp"
#include <entity/player/CPlayer.hpp>
#include <renderer/renderer.hpp>
#include <util/misc.hpp>
#include <engine/engine.hpp>

/*
need to figure out collisions for real

a top down 2d view in editor seems to recurringly be helpful as well

also do something fun because enemies are frustrating

*/


void CWeaponShotgun::Render(CRenderer *renderer)
{
    static constexpr float flx = SCREEN_WIDTH / 17.0667f;
    static constexpr float fly = SCREEN_HEIGHT / -3.6f;
    static constexpr float gx = SCREEN_WIDTH * 0.140625f;
    static constexpr float gy = SCREEN_HEIGHT * 0.0139f;

    auto player = (CPlayer*)(m_pOwner);
    anim_shotgun.Draw(renderer, {0 + player->GetViewModel().GetBobX(), 5.f + 10.f + player->GetViewModel().GetBobY()}); //640x360 = 90, 5   - IEntitySystem->GetLocalPlayer()->GetPosition().z
}

void CWeaponShotgun::OnUpdate()
{
    anim_shotgun.OnUpdate();
 
    //need to add states & animation states
}

void CWeaponShotgun::OnShoot()
{
    m_clip = 0;
   anim_shotgun.PlaySequenceByName("shoot0", true);
  
     engine->SoundSystem()->PlaySound("shotgun_shoot", 0.9); 
}

void CWeaponShotgun::OnReload()
{
    anim_shotgun.PlaySequenceByName("reload0");
     engine->SoundSystem()->PlaySound("shotgun_reload", 0.9); 
    //sound

    //2 options here
    //add weapon spread 
    //orrrr
    //say it uses slugs
    /*
    best way to do it
    - is within FOV
    - is visible
        - so bascially just use enemy util code
    */
}

void CWeaponShotgun::ApplyRecoil()
{
    //spread next!! 
    auto player = (CPlayer*)m_pOwner;
    auto cam = player->m_pCamera();

    double shotsMod = Random::AddVariation<double>( pow(m_shotsFired, m_recoil.shots_fired_mul), 0.3, 0.8);

    double newPitch = cam->m_flPitch +  m_recoil.pitch_per_shot *  shotsMod ;
    if(newPitch >= CCamera::MaxPitch()) newPitch = CCamera::MaxPitch() - Random::Range<double>(0.4, 1.5); ///maybe rand range so it looks better

    cam->m_flPitch = newPitch;
    double newYaw = m_recoil.yaw_per_shot * IInputSystem->GetMouseScale() * shotsMod;
    if(m_shotsFired > (m_data.iMaxAmmo / 2.5) ) newYaw *= -1.0;
    cam->Rotate(newYaw);
}

float CWeaponShotgun::GetDamage(CBaseEntity *ent) const
{
    if(ent)
    {
        float base_dmg = m_data.GetDamage();
        double dist = (m_pOwner->GetPosition() - ent->GetPosition()).Length2D();
        if(dist <= m_data.range){
            return base_dmg;
        }

        return base_dmg * (m_data.range / dist) * 2.f; //two bullets
    }
    return 0.0f;
}

void CWeaponShotgun::OnCreate()
{
    // setup data
    this->m_nNextShot = 0;
    this->m_nFireRate = 15;
    this->m_data.flDamage = 30.0;
    this->m_data.iDamageMod = 11;
    this->m_data.iReloadTime = 87;
    this->m_clip = this->m_data.iMaxAmmo = 2;
    this->m_data.nAmmoType = 2;
    this->m_reserveammo =  this->m_data.iMaxAmmo * 4;
    this->m_data.range = 0.95;

    this->m_recoil.pitch_per_shot = 3.3;
    this->m_recoil.yaw_per_shot = 1.4;
    this->m_recoil.rand = {0.1, 1.2};
    this->m_recoil.shots_fired_mul = 1.0;

    anim_shotgun.AddDefaultSequenceByName("default0", {256, 256}); //640x360 = 450x234
    anim_shotgun.AddSequenceByName("shoot0");
   // anim_shotgun.AddSequenceByName("equip0");
    anim_shotgun.AddSequenceByName("reload0");

    

   
    if (m_pOwner == nullptr)
    {
        log("yo im a gun and im having some fuckin issues finding out who i belong to");
        return;
    }
    assert(m_pOwner->IsLocalPlayer());
}
