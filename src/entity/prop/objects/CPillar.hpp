#pragma once

#include "../CStaticProp.hpp"


class CPillar : public CStaticProp
{
public:
    CPillar(int m_iID) : CStaticProp(m_iID) {}
    virtual ~CPillar() {}
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void CreateRenderable();
    virtual void Render(CRenderer *renderer);

private:
};
void CPillar::OnCreate()
{
    SET_ENT_SUBNAME();

   CreateRenderable();
}

void CPillar::OnDestroy()
{
}

void CPillar::CreateRenderable()
{
    SetupTexture("pillar.png");
}

void CPillar::Render(CRenderer *renderer)
{
   DrawProp(renderer);
}
