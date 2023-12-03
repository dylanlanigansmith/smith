#pragma once
#include <logger/logger.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/components/animation/CAnimationController.hpp>

#include <entity/components/pathfinder/CPathFinder.hpp>


class CBaseEnemy : public CBaseRenderable, public CLogger
{
public:
    CBaseEnemy(int m_iID) : CBaseRenderable(m_iID), CLogger(this, std::to_string(m_iID)), path(CPathFinder(this)) {  }
    virtual ~CBaseEnemy() {}
    virtual void CreateRenderable();
    virtual void OnRenderStart();
    virtual void OnRenderEnd();
    virtual void Render(CRenderer* renderer);
    virtual void OnUpdate();
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void OnHit(int damage, int position);

    auto& GetBBox()  { UpdateBBox(); return m_bbox; } //probably dont need to update
    virtual Vector2 GetBounds() { return m_vecBounds; } //x = width, y = radius
    virtual void Freeze(bool set) { m_bFrozen = set; }
protected:
    virtual void SetupTexture(const std::string& name);
    virtual void DrawEnemy(CRenderer* renderer, double wScale = 1.0, double vScale = 1.0, int vOffset = 0.0);
    virtual void SetUpAnimation();
    virtual void CreateMove(IVector2 dir);
    virtual void OnSetPosition(const Vector2& old_pos, const Vector2& new_pos);
    

    virtual inline void UpdateBBox();
protected:
    IVector2 m_lastRenderPos;
    IVector2 m_lastAnimOffset;
    BBoxAABB m_lastRenderBounds;
    bool m_bFrozen = false;
    IVector2 m_vecNextPoint;
    int m_iLastRenderHeight;
    CAnimController* m_anim;
    Vector2 m_vecBounds;
    BBoxAABB m_bbox;
    int m_iHealth;
    int m_iMaxHealth;
    double m_flMoveSpeed;
    CPathFinder path;
private:
    uint8_t m_state;


};

