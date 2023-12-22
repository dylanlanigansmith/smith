#pragma once
#include  <entity/weapon/CBaseWeapon.hpp>
#include <util/rtti.hpp>
#include <types/CTime.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>
#include <anim/viewmodel/AnimViewmodel.hpp>
class CPlayer;

class CWeaponPistol : public CBaseWeapon
{
public:
    CWeaponPistol(CBaseRenderable* m_pOwner) : CBaseWeapon(m_pOwner, Util::getClassName(this)), anim_pistol(m_pOwner, "pistol"), anim_flash(m_pOwner, "muzzleflash")  {}
    virtual ~CWeaponPistol(){}
    virtual void OnCreate();
    virtual void Render(CRenderer* renderer);
    virtual void OnUpdate();
    virtual void OnShoot();
    virtual void OnReload();
protected:

private:


   CAnimViewmodel anim_pistol;
   CAnimViewmodel anim_flash;
};
