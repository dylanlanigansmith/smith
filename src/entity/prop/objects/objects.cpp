#include "CBarrel.hpp"
#include "CPillar.hpp"
#include "CGreenLight.hpp"

REGISTER_DEF_ENT(CBarrel);
REGISTER_DEF_ENT(CPillar);
REGISTER_DEF_ENT(CGreenLight);

//MAKE THESE GENERIC 


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
