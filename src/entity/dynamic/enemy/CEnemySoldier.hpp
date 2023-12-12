#pragma once
#include "../CBaseEnemy.hpp"
#include <anim/directional/AnimDirectional.hpp>
#include <entity/CMove.hpp>

struct EntView
{
    Vector2 m_plane; //probs not needed
    Vector2 m_dir;

    EntView() : m_plane(0,0.66), m_dir(-1, 0) {}
    

    void rotate(double deg){ m_dir.rotate(deg); }
    void lookAt(const Vector2& pos, const Vector2& target){
        auto delta = target - pos;
        m_dir = delta.Normalize();
    }
    bool lookAt(const Vector2& pos, const Vector2& target, double speed, double close_enough = 4.0){
        Vector2 targetDirection = (target - pos).Normalize();
        m_dir = m_dir.lerp(targetDirection, speed);
         double dot = m_dir.dotClamped(targetDirection);
       
        double angle = std::acos(dot) * RAD2DEGNUM; 

        // Check if the angle is within the threshold
        return angle <= close_enough;

    }
};




enum RelDir : int
{
    Rel_Fwd = 0,
    Rel_Left = 1,
    Rel_Right = 2,
    Rel_Back = 3
};


class CEnemySoldier : public CBaseRenderable //CBaseDynamicEntity 
{
public:
    CEnemySoldier(int m_iID) : CBaseRenderable(m_iID), m_anim(this, "soldier"), m_state(SoldierState::Default) {}
    ~CEnemySoldier() { delete m_Texture; }
    virtual void OnUpdate();
    virtual void OnCreate();
    virtual void OnDestroy() {}
 
    virtual void CreateRenderable();
    virtual void OnRenderStart(){}
    virtual void OnRenderEnd() {}
    virtual void Render(CRenderer* renderer);
    
    virtual void OnHit(int damage, int position) {}

    virtual void WalkTowards(const Vector2& pos);
    int DeduceSequenceForOrientation(int* flip, int* anim_state, int* frame, std::string& seq_name);
protected:
    CMove m_move;

    EntView m_view;
    CAnimDirectional m_anim;
     int m_state;
    sprite_draw_data draw;
public:
    enum SoldierState : int
    {
        Default = 0,
        Standing = 0,
        Walking = 1,
        Running = 1,
        Attacking,
        Dying,
        Dead
    };


};