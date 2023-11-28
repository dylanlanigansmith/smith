#pragma once

#include <common.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/CBaseEntity.hpp>
#include "CCamera.hpp"




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
    const CCamera& Camera() { return m_camera; }
   
private:
    CCamera m_camera;
};

