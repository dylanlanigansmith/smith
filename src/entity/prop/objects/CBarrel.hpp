#pragma once

#include "CStaticProp.hpp"
#include <engine/engine.hpp>

class CBarrel : public CStaticProp
{
public:
    CBarrel(int m_iID) : CStaticProp(m_iID) {}
    virtual ~CBarrel() {}
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void CreateRenderable();
    virtual void Render(CRenderer* renderer);
private:
   
};
void CBarrel::OnCreate()
{
    SET_ENT_SUBNAME();

    auto ITextureSystem = engine->CreateInterface<CTextureSystem>("ITextureSystem"); //this isnt good
    m_hTexture = ITextureSystem->LoadTexture("barrel.png");
    
}

void CBarrel::OnDestroy()
{
    
}

void CBarrel::CreateRenderable()
{
    
}

void CBarrel::Render(CRenderer* renderer)
{
    
}

