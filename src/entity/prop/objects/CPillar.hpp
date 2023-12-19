#pragma once

#include "../CStaticProp.hpp"

#include ENTREG_INC
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
    REGISTER_DEC_ENT(CPillar);
};

