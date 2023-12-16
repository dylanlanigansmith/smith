#pragma once

#include "CBaseProp.hpp"

class CStaticProp : public CBaseProp
{
public:
    CStaticProp(int m_iID) : CBaseProp(m_iID) {}
    virtual ~CStaticProp() {}
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void CreateRenderable() = 0;
    virtual void Render(CRenderer* renderer) = 0;
private:
   
};

/*
need to serialize to json with standard sprite params and texture name saved and reigster to types like lighting

*/