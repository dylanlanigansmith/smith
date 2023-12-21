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
  void Rotate(double rotSpeed) // positive CCW/L negative CW/R
  {
    // both camera direction and camera plane must be rotated
    double oldplayerDirX = m_vecDir.x;
    m_vecDir.x = m_vecDir.x * cos(rotSpeed) - m_vecDir.y * sin(rotSpeed);
    m_vecDir.y = oldplayerDirX * sin(rotSpeed) + m_vecDir.y * cos(rotSpeed);
    double oldplaneX = m_vecPlane.x;
    m_vecPlane.x = m_vecPlane.x * cos(rotSpeed) - m_vecPlane.y * sin(rotSpeed);
    m_vecPlane.y = oldplaneX * sin(rotSpeed) + m_vecPlane.y * cos(rotSpeed);
  }
  enum class W2S
  {
    Top = 0,
    Middle,
    Bottom
  };
  IVector2 WorldToScreen(Vector2 world, W2S pos = W2S::Top , int vOffset = 0, double vScale = 1.0, double wScale = 1.0)
  {
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
    screen.x = int((SCREEN_WIDTH / 2) * (1 + (transform.x / transform.y)));
    screen.y = transform.y; // IDK

    // if pitch
    //  int vMoveScreen = int(vOffset / transform.y) + camera->m_flPitch + (camera->m_vecPosition.z / transform.y);
    int vMoveScreen = int(vOffset / transform.y) + m_flPitch + (m_vecPosition.z / transform.y);
    int renderWidth = abs(int(SCREEN_HEIGHT / (transform.y))) / wScale;
    int renderHeight = std::abs(int(SCREEN_HEIGHT / (transform.y))) / vScale;
    int drawStartY = (-renderHeight / 2) + (SCREEN_HEIGHT / 2) + vMoveScreen;
    if (drawStartY < 0)
    {
      drawStartY = 0;
    }

    int drawEndY = (renderHeight / 2) + (SCREEN_HEIGHT / 2) + vMoveScreen;
    if (drawEndY >= SCREEN_HEIGHT)
      drawEndY = SCREEN_HEIGHT - 1;
     int drawStartX = (-renderWidth / 2) + screen.x;
    if (drawStartX < 0)
        drawStartX = 0;
    int drawEndX = (renderWidth / 2) + screen.x;
    if (drawEndX >= SCREEN_WIDTH)
        drawEndX = SCREEN_WIDTH - 1;

    int dx, dy;
    switch(pos)
    {
      case W2S::Bottom:
        dx = drawEndX;
        dy = drawEndY;
        break;
      case W2S::Middle:
        dx = ((drawEndX - drawStartX) / 2) + drawStartX;
        dy = ((drawEndY - drawStartY) / 2) + drawStartY;
        break;
      case W2S::Top:
      default:
        dx = drawStartX;
        dy = drawStartY;

    };   
    screen.x = (dx ) * (SCREEN_WIDTH_FULL / SCREEN_WIDTH);
    screen.y = (dy) * (SCREEN_HEIGHT_FULL / SCREEN_HEIGHT);
    if(screen.x > SCREEN_WIDTH_FULL || screen.x <= 0)
      screen.x = -1;
    if(screen.y > SCREEN_HEIGHT_FULL || screen.y <= 0)
      screen.y = -1;
    return screen;
  }
};