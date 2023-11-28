#pragma once

#include <common.hpp>
#include <renderer/renderer.hpp>
#include <interfaces/ITextureSystem/ITextureSystem.hpp>
#include "CBaseEntity.hpp"

class CBaseRenderable :  public CBaseEntity
{
public:
    CBaseRenderable(int m_iID) : CBaseEntity(m_iID) {}
    virtual ~CBaseRenderable(){}
    virtual bool IsRenderable() { return true; }
    virtual void CreateRenderable() = 0;
    virtual void OnRenderStart() = 0;
    virtual void OnRenderEnd() = 0;
    virtual void Render(CRenderer* renderer) = 0;
    virtual hTexture GetTexture() { return m_hTexture; }
protected:
    hTexture m_hTexture;
};
