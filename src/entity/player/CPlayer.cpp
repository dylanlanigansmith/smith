#include "CPlayer.hpp"
#include <engine/engine.hpp>
#include <interfaces/IInputSystem/IInputSystem.hpp>

CPlayer::CPlayer(int m_iID) : CBaseRenderable(m_iID)
{
}

CPlayer::~CPlayer()
{
}

void CPlayer::OnUpdate()
{
}

void CPlayer::OnCreate()
{
  SET_ENT_NAME();
  SET_ENT_TYPE();

  m_vecPosition = Vector(22, 12, 0);
  m_camera.m_vecDir = {-1, 0};
  m_camera.m_vecPlane = {0, 0.66};
  m_camera.m_flPitch = 0.0;
}

void CPlayer::OnDestroy()
{
}

void CPlayer::CreateRenderable()
{
}

void CPlayer::OnRenderStart()
{
}

void CPlayer::OnRenderEnd()
{
  static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
  static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
  double frameTime = IEngineTime->GetLastFrameTime().sec() / 200.f; // ticks bro u need ticks
  double moveSpeed = frameTime * 5.0;                               // the constant value is in squares/second
  double rotSpeed = frameTime * 3.0;                                // the constant value is in radians/second
  double pitchSpeed = frameTime * 3.0;                              // the constant value is in radians/second

  WASD_t m_move = IInputSystem->GetInput();
  if (m_move.w)
  {
    if (ILevelSystem->GetMapAt(m_vecPosition.x + Camera().m_vecDir.x * moveSpeed, int(m_vecPosition.y)) == false)
      m_vecPosition.x += Camera().m_vecDir.x * moveSpeed;
    if (ILevelSystem->GetMapAt(int(m_vecPosition.x), int(m_vecPosition.y + Camera().m_vecDir.y * moveSpeed)) == false)
      m_vecPosition.y += Camera().m_vecDir.y * moveSpeed;
  }
  // move backwards if no wall behind you
  if (m_move.s)
  {
    if (ILevelSystem->GetMapAt(int(m_vecPosition.x - Camera().m_vecDir.x * moveSpeed), int(m_vecPosition.y)) == false)
      m_vecPosition.x -= Camera().m_vecDir.x * moveSpeed;
    if (ILevelSystem->GetMapAt(int(m_vecPosition.x), int(m_vecPosition.y - Camera().m_vecDir.y * moveSpeed)) == false)
      m_vecPosition.y -= Camera().m_vecDir.y * moveSpeed;
  }
  // rotate to the right
  if (m_move.d)
  {
    // both camera direction and camera plane must be rotated
    double oldplayerDirX = m_camera.m_vecDir.x;

    m_camera.m_vecDir.x = m_camera.m_vecDir.x * cos(-rotSpeed) - m_camera.m_vecDir.y * sin(-rotSpeed);
    m_camera.m_vecDir.y = oldplayerDirX * sin(-rotSpeed) + m_camera.m_vecDir.y * cos(-rotSpeed);
    double oldplaneX = m_camera.m_vecPlane.x;
    m_camera.m_vecPlane.x = m_camera.m_vecPlane.x * cos(-rotSpeed) - m_camera.m_vecPlane.y * sin(-rotSpeed);
    m_camera.m_vecPlane.y = oldplaneX * sin(-rotSpeed) + m_camera.m_vecPlane.y * cos(-rotSpeed);
  }
  // rotate to the left
  if (m_move.a)
  {
    // both camera direction and camera plane must be rotated
    double oldplayerDirX = m_camera.m_vecDir.x;
    m_camera.m_vecDir.x = m_camera.m_vecDir.x * cos(rotSpeed) - m_camera.m_vecDir.y * sin(rotSpeed);
    m_camera.m_vecDir.y = oldplayerDirX * sin(rotSpeed) + m_camera.m_vecDir.y * cos(rotSpeed);
    double oldplaneX = m_camera.m_vecPlane.x;
    m_camera.m_vecPlane.x = m_camera.m_vecPlane.x * cos(rotSpeed) - m_camera.m_vecPlane.y * sin(rotSpeed);
    m_camera.m_vecPlane.y = oldplaneX * sin(rotSpeed) + m_camera.m_vecPlane.y * cos(rotSpeed);
  }
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_UP))
  {
    // look up
    m_camera.m_flPitch += 400 * pitchSpeed;
    if (m_camera.m_flPitch > 200)
      m_camera.m_flPitch = 200;
  }
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_DOWN))
  {
    // look down
    m_camera.m_flPitch -= 400 * pitchSpeed;
    if (m_camera.m_flPitch < -200)
      m_camera.m_flPitch = -200;
  }
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_SPACE))
  {
    // jump
    m_vecPosition.z = 200;
  }
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_LSHIFT))
  {
    // crouch
    m_vecPosition.z = -200;
  }
  if (m_camera.m_flPitch > 0)
    m_camera.m_flPitch = std::max<double>(0, m_camera.m_flPitch - 100 * moveSpeed);
  if (m_camera.m_flPitch < 0)
    m_camera.m_flPitch = std::min<double>(0, m_camera.m_flPitch + 100 * moveSpeed);
  if (m_vecPosition.z > 0)
    m_vecPosition.z = std::max<double>(0, m_vecPosition.z - 100 * moveSpeed);
  if (m_vecPosition.z < 0)
    m_vecPosition.z = std::min<double>(0, m_vecPosition.z + 100 * moveSpeed);
}

void CPlayer::Render(CRenderer *renderer)
{
}