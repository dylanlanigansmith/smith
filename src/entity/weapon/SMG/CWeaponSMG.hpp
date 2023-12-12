#pragma once
#include  <entity/weapon/CBaseWeapon.hpp>
#include <util/rtti.hpp>
#include <types/CTime.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>
#include <anim/viewmodel/AnimViewmodel.hpp>
class CPlayer;

class CWeaponSMG : public CBaseWeapon
{
public:
    CWeaponSMG(CBaseRenderable* m_pOwner) : CBaseWeapon(m_pOwner, Util::getClassName(this)), anim_smg(m_pOwner, "smg"){}
    virtual ~CWeaponSMG(){}
    virtual void OnCreate();
    virtual void Render(CRenderer* renderer);
    virtual void OnUpdate();
    virtual void OnShoot();
    virtual void OnReload();
protected:

private:
   CAnimController* m_flash;

   CAnimViewmodel anim_smg;
  
};
