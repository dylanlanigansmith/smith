#pragma once

#include "../CStaticProp.hpp"



#include ENTREG_INC


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
     REGISTER_DEC_ENT(CBarrel);
};
