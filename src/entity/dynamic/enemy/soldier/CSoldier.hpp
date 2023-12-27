#pragma once
#include <entity/dynamic/CDirectionalEntity.hpp>
#include <entity/dynamic/CBaseEnemy.hpp>
#include <entity/components/AI/CBehaviour.hpp>
#include ENTREG_INC
enum SoldierTypes : int
{
    Soldier_Grunt = 0,
    Soldier_Med = 1,
    Soldier_Command = 3
};

class CSoldier : public CBaseEnemy, public CDirectionalEntity, protected CLogger
{
    friend class CEditor;
public:
    CSoldier(int m_ID) :  CDynamicEntity(m_ID), CBaseEnemy(m_ID), CDirectionalEntity(m_ID, "soldier"),  CLogger(this, (std::string)this->m_iID), m_behave(this) {}
    virtual ~CSoldier(){}

    virtual void OnUpdate();
    virtual void OnCreate();
    virtual void OnDestroy();


    virtual void CreateRenderable();
    virtual void Render(CRenderer* renderer);

    virtual bool Attack(CBaseEntity* target) { return Shoot(target);  }

    virtual void OnHit(int damage, const IVector2& position);
    virtual bool HasLoot() const { return !m_loot.m_used; }

    virtual bool HitDetect(CCamera* camera, const IVector2 &point, IVector2 *textpos = nullptr); 
    virtual float GetDamageModForHit(const IVector2& pt);
    virtual void SetSubType(int type = 0){
        switch(type)
        {
            case Soldier_Command:
                 m_move.m_flForwardSpeed *= 1.25f;
           //     m_anim.ChangeBaseTexture("soldier3.png"); 
                    break;
            case Soldier_Med:
                m_move.m_flForwardSpeed /= 1.5f;
                m_combat.m_damage_primary -= 3; m_combat.m_damage_variable += 2;
                 m_health = m_maxhealth = 70;
                m_anim.ChangeBaseTexture("soldier2.png"); break;
            case Soldier_Grunt:   
            default:
                m_health = m_maxhealth = 30;
                 m_move.m_flForwardSpeed *= 1.7f;
                m_combat.m_damage_primary  += 4; m_combat.m_damage_variable  += 4;
                m_anim.ChangeBaseTexture("soldier.png"); break;
              
        }
    }
    int DeduceSequenceForOrientation(int* flip, int* anim_state, int* frame, std::string& seq_name);

    

protected:
    virtual bool Shoot(CBaseEntity* target);
    virtual void OnDeath();
    virtual Color GetPixelAtPoint(CCamera* camera, const IVector2 &point, IVector2 *textpos = nullptr); 

    virtual void CreateBehaviours();
     virtual void OnWalkToward(const Vector2& pos);
     virtual void OnLikelyStuck(); 
protected:
    CBehaviourControl m_behave;

    double m_repositionrange;

    uint64_t m_lastVocal;
    uint64_t m_lastFootstep;
private:
    REGISTER_DEC_ENT(CSoldier);
};