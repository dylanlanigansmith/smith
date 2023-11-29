#pragma once

#include "../CStaticProp.hpp"


class CBarrel : public CStaticProp
{
public:
    CBarrel(int m_iID) : CStaticProp(m_iID) {}
    virtual ~CBarrel() {}
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void CreateRenderable();
    virtual void Render(CRenderer *renderer);

private:
};
void CBarrel::OnCreate()
{
    SET_ENT_SUBNAME();

   CreateRenderable();
}

void CBarrel::OnDestroy()
{
}

void CBarrel::CreateRenderable()
{
    SetupTexture("barrel.png");
}

void CBarrel::Render(CRenderer *renderer)
{
   DrawProp(renderer);
}
