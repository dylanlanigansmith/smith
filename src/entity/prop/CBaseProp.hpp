#pragma once

#include <common.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/CBaseEntity.hpp>
#include <engine/engine.hpp>

//uisng old texture system

class CBaseProp : public CBaseRenderable
{
public:
    CBaseProp(int m_iID);
    virtual ~CBaseProp() {}
    virtual void OnUpdate() {}
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void CreateRenderable() = 0;
    virtual void OnRenderStart() {} 
    virtual void OnRenderEnd() {}
    virtual void Render(CRenderer* renderer) = 0;
protected:
    virtual void SetupTexture(const std::string& name);
    virtual void DrawProp(CRenderer* renderer, double wScale = 1.0, double vScale = 1.0, int vOffset = 0.0);
};

