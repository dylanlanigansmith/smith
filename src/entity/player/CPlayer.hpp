#pragma once

#include <common.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/CBaseEntity.hpp>
#include "CCamera.hpp"

#include <entity/weapon/pistol/CWeaponPistol.hpp>


class CPlayer : public CBaseRenderable
{
    friend class CRenderer;
public:
    CPlayer(int m_iID);
    virtual ~CPlayer();
    virtual void OnUpdate();
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void CreateRenderable();
    virtual void OnRenderStart();
    virtual void OnRenderEnd();
    virtual void Render(CRenderer* renderer);
    virtual bool IsLocalPlayer() { return true;}
    CCamera Camera() const { return m_camera; }

    CCamera* m_pCamera() { return &m_camera; } //not a good system
private:
    virtual void CreateMove();

    CBaseWeapon* GetActiveWeapon() { return m_inventory.at(m_nActiveWeapon); }
private:
    uint8_t m_nActiveWeapon = 0;
    std::vector<CBaseWeapon*> m_inventory;
    CCamera m_camera;
};

