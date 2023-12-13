#pragma once
#include "../CBaseEnemy.hpp"
#include <anim/directional/AnimDirectional.hpp>
#include <entity/CMove.hpp>
#include <entity/components/pathfinder/CPathFinder.hpp>

//#define IGNORE_PLAYER

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


struct foe_info
{
    int m_health;
    int m_maxhealth;
    int m_main_damage;
    int m_alt_damage;

    foe_info(int m_maxhealth) : 
        m_health(m_maxhealth), m_maxhealth(m_maxhealth), m_main_damage(10), m_alt_damage(5){}
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
    friend class CEditor;
public:
    CEnemySoldier(int m_iID) : CBaseRenderable(m_iID), m_stats(50),  m_anim(this, "soldier"), m_state(SoldierState::Default), m_path(this) {}
    ~CEnemySoldier() { delete m_Texture; }
    virtual void OnUpdate();
    virtual void OnCreate();
    virtual void OnDestroy() {}
 
    virtual void CreateRenderable();
    virtual void OnRenderStart(){}
    virtual void OnRenderEnd() {}
    virtual void Render(CRenderer* renderer);
    
    virtual void OnHit(int damage, int position) {
        m_behaviour = Behaviour_Aiming;
        m_state = Aiming;
        m_stats.m_health -= damage;
        if(m_stats.m_health <= 0)
            m_state = Dying;
    }

    auto GetHealth() const { return m_stats.m_health; }
    auto GetMaxHealth() const { return m_stats.m_maxhealth; }
    auto GetPathFinder() { return &m_path; }
    virtual void WalkTowards(const Vector2& pos);
    int DeduceSequenceForOrientation(int* flip, int* anim_state, int* frame, std::string& seq_name);

    bool isPlayerVisible(CPlayer* player, double fov = 40.0);
    bool isPlayerWithinFOV( const Vector2 &playerPosition, double FOVAngleDegrees);
    bool CastRayToPlayer(const Vector2 &playerPosition, float playerBounds);
    virtual void Shoot(CPlayer* player);

    virtual uint32_t GetPixelAtPoint( CCamera* camera, const IVector2 &point, IVector2* textpos);
    virtual void SetType(int type = 0){
        switch(type)
        {
            case Soldier_Command:
                m_anim.ChangeBaseTexture("soldier3.png"); break;
            case Soldier_Med:
                m_anim.ChangeBaseTexture("soldier2.png"); break;
            case Soldier_Grunt:   
            default:
                m_anim.ChangeBaseTexture("soldier.png"); break;
              
        }
    }
protected:
    int m_nextBehaviour;
    int m_behaviour;
    uint64_t m_nextBehaviourChange;
    foe_info m_stats;
    CMove m_move;

    EntView m_view;
    CAnimDirectional m_anim;
     int m_state;
    sprite_draw_data draw;

    CPathFinder m_path;
public:
    enum SoldierBehaviour : int 
    {
        Behaviour_Default = 0,
        Behaviour_Patrol,
        Behaviour_Aiming,
        Behaviour_Attack,
        Behaviour_Reposition,
        Behaviour_PostAttack,
    };

    enum SoldierState : int
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

    enum SoldierTypes : int
    {
        Soldier_Grunt = 0,
        Soldier_Med = 1,
        Soldier_Command = 3
    };
};