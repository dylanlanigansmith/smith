#pragma once
#include  <entity/weapon/CBaseWeapon.hpp>
#include <util/rtti.hpp>
#include <types/CTime.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>

class CPlayer;

class CWeaponPistol : public CBaseWeapon
{
public:
    CWeaponPistol() : CBaseWeapon(Util::getClassName(this)) {}
    virtual ~CWeaponPistol(){}
    virtual void OnCreate();
    virtual void Render(CRenderer* renderer);
    virtual void OnUpdate();
    virtual void Shoot();
protected:
    virtual bool HitDetect(CPlayer* player, CBaseEnemy* ent, const Vector2& rayDir);
    virtual bool HitDetect2(CPlayer* player, CBaseEnemy* ent, const Vector2& rayDir);
    virtual bool HitDetectPixelPerfect(CPlayer* player, CBaseEnemy* ent, IVector2* textpos);
    virtual int FindTexturePoint(CPlayer* player, CBaseEnemy* ent, const Vector2& rayDir);
private:
   CAnimController* m_flash;
};
