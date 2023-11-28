#pragma once

#include <common.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/CBaseEntity.hpp>





class CBaseProp : public CBaseRenderable
{
public:
    CBaseProp(int m_iID);
    virtual ~CBaseProp() {}
    virtual void OnUpdate() {}
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void CreateRenderable() = 0;
    virtual void OnRenderStart() {} 
    virtual void OnRenderEnd() {}
    virtual void Render(CRenderer* renderer) = 0;
private:
   
};

CBaseProp::CBaseProp(int m_iID) : CBaseRenderable(m_iID)
{
    ENT_SETUP();
}

