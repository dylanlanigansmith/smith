#pragma once
#include <types/Vector.hpp>

class CCamera
{
public:
    Vector m_vecPosition;
    Vector2 m_vecDir;
    Vector2 m_vecPlane;
    double m_flPitch;
    double m_bobAmt;
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

    IVector2 WorldToScreen(Vector2 world){
       Vector2 relPos = {
         world.x - m_vecPosition.x, 
        world.y - m_vecPosition.y, 
     };
      

      const auto camPlane = m_vecPlane;
      const auto camDir = m_vecDir;
      double invDet = 1.0 / ((camPlane.x * camDir.y) - (camDir.x * camPlane.y));
  
      Vector2 transform;
      // transform sprite with the inverse camera matrix
      //  [ planeX   dirX ] -1                                       [ dirY      -dirX ]
      //  [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
      //  [ planeY   dirY ]                                          [ -planeY  planeX ]
      transform.x = invDet * (camDir.y * relPos.x - camDir.x * relPos.y);
      transform.y = invDet * (-1.0 * (camPlane.y) * relPos.x + camPlane.x * relPos.y);

      IVector2 screen;
      screen.x = int((SCREEN_WIDTH / 2) * (1 + (transform.x / transform.y) ));  
      screen.y = transform.y; //IDK

    //if pitch
    // int vMoveScreen = int(vOffset / transform.y) + camera->m_flPitch + (camera->m_vecPosition.z / transform.y);


      int renderHeight = std::abs(int(SCREEN_HEIGHT / (transform.y))) ;
      screen.y = renderHeight;
      return screen;
    }
    
};