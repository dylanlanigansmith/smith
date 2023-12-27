#include "CDirectionalEntity.hpp"

#include <engine/engine.hpp>

void CDirectionalEntity::WalkTowards(const Vector2 &pos)
{
     double moveSpeed = m_move.m_flForwardSpeed;
    if(!m_view.lookAt(m_vecPosition, pos, m_move.m_flYawSpeed))
        moveSpeed /= 2.0;
    
    m_action = Walking;
    Vector newPos = m_vecPosition;
    bool wasCol = false;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x + m_view.m_dir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false )
        newPos.x += m_view.m_dir.x * moveSpeed;
    else wasCol = true;

    if(ILevelSystem->IsCollision(this, newPos, {newPos.x,  newPos.y + m_view.m_dir.y * moveSpeed, newPos.z}) == false)
        newPos.y += m_view.m_dir.y * moveSpeed;
    else wasCol = true;
    
    SetPosition(newPos);
    
    if ( m_vecPosition.x != newPos.x || m_vecPosition.y != newPos.y  || wasCol ){
        OnLikelyStuck();
    }
    else  OnWalkToward(pos);

}

void CDirectionalEntity::RunTowards(const Vector2 &pos)
{
    double oldSpeed = m_move.m_flForwardSpeed;
    m_move.m_flForwardSpeed *= m_move.m_flSpeedModifier;
    WalkTowards(pos);
    m_move.m_flForwardSpeed = oldSpeed;
    //lazy
}
