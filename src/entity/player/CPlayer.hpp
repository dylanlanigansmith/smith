#pragma once

#include <common.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/CBaseEntity.hpp>
#include "CCamera.hpp"
#include <entity/weapon/CBaseWeapon.hpp>
#include <entity/weapon/weapons.hpp>
#include "../CMove.hpp"
#include "CViewmodel.hpp"
#include "CInventory.hpp"



class CPlayer : public CBaseRenderable
{
    friend class CRenderer; friend class CEditor; friend class CViewmodel;
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
    virtual void RenderView(CRenderer* renderer); //not lit stuff
    virtual bool IsLocalPlayer() { return true;}
    CCamera Camera() const { return m_camera; }

    CCamera* m_pCamera() { return &m_camera; } //not a good system

    virtual void OnHit(int damage);

    auto GetHealth() const { return m_health; }
    virtual float GetBounds() const override { return 0.25f; }
    virtual void OnCollisionWith(CBaseEntity* hit) override;
private:
    int m_health;
    int m_max_health;
    virtual void CreateMove();
    CMove m_move;
    CBaseWeapon* GetActiveWeapon() { return m_inventory->Active(); }
    auto& Inventory() { return m_inventory; }
    CViewmodel m_viewmodel;
    inventory_t* m_inventory;
private:
    uint8_t m_nActiveWeapon = 0;
    CCamera m_camera;
};

