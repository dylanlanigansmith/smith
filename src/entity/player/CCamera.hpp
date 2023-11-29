#pragma once
#include <types/Vector.hpp>

class CCamera
{
public:
    Vector m_vecPosition;
    Vector2 m_vecDir;
    Vector2 m_vecPlane;
    double m_flPitch;
    void Rotate(double rotSpeed) //positive CCW/L negative CW/R
    {
      // both camera direction and camera plane must be rotated
      double oldplayerDirX = m_vecDir.x;
      m_vecDir.x = m_vecDir.x * cos(rotSpeed) - m_vecDir.y * sin(rotSpeed);
      m_vecDir.y = oldplayerDirX * sin(rotSpeed) + m_vecDir.y * cos(rotSpeed);
      double oldplaneX = m_vecPlane.x;
      m_vecPlane.x = m_vecPlane.x * cos(rotSpeed) - m_vecPlane.y * sin(rotSpeed);
      m_vecPlane.y = oldplaneX * sin(rotSpeed) + m_vecPlane.y * cos(rotSpeed);
    }

    
};