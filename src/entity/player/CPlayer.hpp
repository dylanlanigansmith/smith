#pragma once

#include <common.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/CBaseEntity.hpp>



class CCamera
{
public:
    Vector2 m_vecDir;
    Vector2 m_vecPlane;
    double m_flPitch;
};

class CPlayer : public CBaseRenderable
{
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
    const CCamera& Camera() { return m_camera; }
   
private:
    CCamera m_camera;
};

