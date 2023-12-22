#pragma once

#include "CBaseEnemy.hpp"
#include "CDynamicEntity.hpp"
#include <anim/directional/AnimDirectional.hpp>
#include <entity/CMove.hpp>
#include "EntView.hpp"

/*
CHumanEnemy

-CHumanoid
-Has 8D directional anim

*/

class CDirectionalEntity : virtual public CDynamicEntity
{
public:
    enum ActionTypes : int
    {
        Default = 0,
        Standing = 0,
        Walking = 1,
        Running = 1,
        Aiming,
        Attacking,
        Dying,
        Dead
    };
    CDirectionalEntity(int m_iID, const std::string& m_animname) : CDynamicEntity(m_iID), m_anim(this, m_animname) {  }
    virtual ~CDirectionalEntity() {}

    


     auto& GetAnim() { return m_anim; }
    auto GetCurrentAction() const { return m_action; }
    virtual bool HasTexture() { return false; }
protected:
    inline void SetAction(int act) { m_action = act; }
    virtual void WalkTowards(const Vector2& pos);
    virtual void RunTowards(const Vector2& pos);
    virtual void OnWalkToward(const Vector2& pos) {}
    virtual void OnLikelyStuck() {}
protected:

   CAnimDirectional m_anim;
    
   int m_action;
};