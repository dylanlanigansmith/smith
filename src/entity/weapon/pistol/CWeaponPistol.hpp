#pragma once
#include  <entity/weapon/CBaseWeapon.hpp>
#include <util/rtti.hpp>
#include <types/CTime.hpp>
enum class Pistol_AnimStates : uint8_t
{
    Anim_Default = 0,
    Anim_Shoot1,
    Anim_Shoot2,

};

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
    SDL_Surface* m_surface;
    Pistol_AnimStates m_animState; //rough design i promise

    Time_t m_lastAnimTime;
    Time_t m_nextAnimTime;
};
