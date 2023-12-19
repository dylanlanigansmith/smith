#pragma once

#include "../CStaticProp.hpp"
#include ENTREG_INC

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
REGISTER_DEC_ENT(CGreenLight);
};
