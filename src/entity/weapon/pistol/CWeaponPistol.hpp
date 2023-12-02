#pragma once
#include  <entity/weapon/CBaseWeapon.hpp>
#include <util/rtti.hpp>
#include <types/CTime.hpp>

class CWeaponPistol : public CBaseWeapon
{
public:
    CWeaponPistol() : CBaseWeapon(Util::getClassName(this)) {}
    virtual ~CWeaponPistol(){}
    virtual void OnCreate();
    virtual void Render(CRenderer* renderer);
    virtual void OnUpdate();
    virtual void Shoot();
private:
   CAnimController* m_flash;
};
