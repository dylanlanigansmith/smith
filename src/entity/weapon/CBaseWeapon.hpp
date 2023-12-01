#pragma once

#include <common.hpp>

#include <entity/CBaseRenderable.hpp>
#include <entity/components/animation/CAnimationController.hpp>

//https://forum.zdoom.org/viewtopic.php?t=71476
//https://forum.zdoom.org/viewtopic.php?t=70605


class CBaseWeapon 
{
public:
    CBaseWeapon(const std::string& m_szName) : m_szName(m_szName) {}
    virtual ~CBaseWeapon(){}
    virtual void OnCreate() = 0;
    virtual void Render(CRenderer* renderer) = 0;
    virtual void OnUpdate() = 0;
    virtual void Shoot() = 0;
    virtual void SetOwnerEntity(hEntity m_iOwnerID) { this->m_iOwnerID = m_iOwnerID; OnSetOwnerEntity();}
    CAnimController* m_anim;
private: 

    void OnSetOwnerEntity();
protected:
    texture_t* m_texture;
    
    hEntity m_iOwnerID;
    CBaseRenderable* m_pOwner = nullptr;
    
    std::string m_szName;
};