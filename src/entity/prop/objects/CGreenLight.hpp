#pragma once

#include "../CStaticProp.hpp"


class CGreenLight : public CStaticProp
{
public:
    CGreenLight(int m_iID) : CStaticProp(m_iID) {}
    virtual ~CGreenLight() {}
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void CreateRenderable();
    virtual void Render(CRenderer *renderer);

private:
};
void CGreenLight::OnCreate()
{
    SET_ENT_SUBNAME();

   CreateRenderable();
}

void CGreenLight::OnDestroy()
{
}

void CGreenLight::CreateRenderable()
{
    SetupTexture("greenlight.png");
}

void CGreenLight::Render(CRenderer *renderer)
{
   DrawProp(renderer, 1.0, 1.0, 128);
}
