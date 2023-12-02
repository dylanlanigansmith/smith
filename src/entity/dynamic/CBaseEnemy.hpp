#pragma once
#include <logger/logger.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/components/animation/CAnimationController.hpp>




class CBaseEnemy : public CBaseRenderable, public CLogger
{
public:
    CBaseEnemy(int m_iID) : CBaseRenderable(m_iID), CLogger(this, std::to_string(m_iID)) {}
    virtual ~CBaseEnemy() {}
    virtual void CreateRenderable();
    virtual void OnRenderStart();
    virtual void OnRenderEnd();
    virtual void Render(CRenderer* renderer);
    virtual void OnUpdate();
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void OnHit(int damage, int position);
protected:
    virtual void SetupTexture(const std::string& name);
    virtual void DrawEnemy(CRenderer* renderer, double wScale = 1.0, double vScale = 1.0, int vOffset = 0.0);
    virtual void SetUpAnimation();
    virtual void CreateMove(IVector2 dir);

    virtual Vector2 GetBounds() { return m_vecBounds; }
protected:
    CAnimController* m_anim;
    Vector2 m_vecBounds;
    int m_iHealth;
    int m_iMaxHealth;
    double m_flMoveSpeed;
private:
    uint8_t m_state;


};

