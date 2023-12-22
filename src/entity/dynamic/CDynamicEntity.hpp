#pragma once

#include <entity/CBaseRenderable.hpp>
#include <entity/CMove.hpp>


class CDynamicEntity : public CBaseRenderable
{
public: 
    CDynamicEntity(int m_iID) : CBaseRenderable(m_iID), m_bounds(0.3f), m_blockingCollisions(true) {}
    virtual ~CDynamicEntity() {}
    virtual float GetBounds() const { return m_bounds; }
    virtual bool IsBlocking() const { return m_blockingCollisions; }
    auto& GetMoveData() const { return m_move; }
   
    auto& GetView() { return m_view; }
protected:
    virtual void SetBlockingCollision(bool s) { m_blockingCollisions = s; }
    virtual void SetBounds(float b) { m_bounds = b; }
protected:
    CMove m_move;
    EntView m_view;
    sprite_draw_data draw;
    
    float m_bounds;
    bool m_blockingCollisions;
};