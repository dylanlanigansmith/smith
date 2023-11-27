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
    virtual bool IsRenderable();
    virtual void OnUpdate();
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void CreateRenderable();
    virtual void OnRenderStart();
    virtual void OnRenderEnd();
    virtual void Render(CRenderer* renderer);
    const CCamera& Camera() { return m_camera; }
    void SetPosition(double x, double y){
        m_vecPosition.x = x;
         m_vecPosition.y = y;
    }
private:
   
    CCamera m_camera;
};

