#pragma once
#include  <entity/weapon/CBaseWeapon.hpp>
#include <util/rtti.hpp>
#include <types/CTime.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>
#include <anim/viewmodel/AnimViewmodel.hpp>
#include <util/random.hpp>

class CPlayer;

class CWeaponShotgun : public CBaseWeapon
{
public:
    CWeaponShotgun(CBaseRenderable* m_pOwner) : CBaseWeapon(m_pOwner, Util::getClassName(this)), anim_shotgun(m_pOwner, "shotgun"){}
    virtual ~CWeaponShotgun(){}
    virtual void OnCreate();
    virtual void Render(CRenderer* renderer);
    virtual void OnUpdate();
    virtual void OnShoot();
    virtual void OnReload();

    virtual void ApplyRecoil();

    virtual float GetDamage(CBaseEntity* ent = nullptr) const override;
protected:

private:
  

   CAnimViewmodel anim_shotgun;
  
};
