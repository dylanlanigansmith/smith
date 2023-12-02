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
  for(auto& item : m_inventory){
    item->OnUpdate();
  }
  CreateMove();

}

void CPlayer::OnCreate()
{
  SET_ENT_NAME();
  SET_ENT_TYPE();

  m_vecPosition = {4,10,0};//Vector(22, 12, 0);
  m_camera.m_vecDir = {-1, 0};
  m_camera.m_vecPlane = {0, 0.66};
  m_camera.m_flPitch = 0.0;

  auto Pistol = new CWeaponPistol();

  m_inventory.push_back(Pistol);
  for(auto& item : m_inventory){
    item->SetOwnerEntity(m_iID);
    item->OnCreate();
  }
    
}

void CPlayer::OnDestroy()
{
  for(auto& item : m_inventory){
    delete item;
  }
}

void CPlayer::CreateRenderable()
{
  

}

void CPlayer::OnRenderStart()
{
  m_camera.m_vecPosition = m_vecPosition;
}

void CPlayer::OnRenderEnd()
{

}

void CPlayer::Render(CRenderer *renderer)
{
    GetActiveWeapon()->Render(renderer);
}

void CPlayer::CreateMove()
{
  static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
  //  static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
  // double frameTime = IEngineTime->GetLastFrameTime().sec() / 50.f; // ticks bro u need ticks
  double frameTime = 0.008;
  double moveSpeed = frameTime * 5.0;  // the constant value is in squares/second
  double rotSpeed = frameTime * 2.5;   // the constant value is in radians/second
  double pitchSpeed = frameTime * 3.5; // the constant value is in radians/second

  WASD_t m_move = IInputSystem->GetInput();
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_LSHIFT))
  {
    // sprint
    moveSpeed *= 1.75f;
  }
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
  if (m_move.d)
  {
    float rightX = Camera().m_vecDir.y;
    float rightY = -Camera().m_vecDir.x;

    if (ILevelSystem->GetMapAt(m_vecPosition.x + rightX * moveSpeed, int(m_vecPosition.y)) == false)
      m_vecPosition.x += rightX * moveSpeed;
    if (ILevelSystem->GetMapAt(int(m_vecPosition.x), m_vecPosition.y + rightY * moveSpeed) == false)
      m_vecPosition.y += rightY * moveSpeed;
  }

  // move left
  if (m_move.a)
  {
    float leftX = -Camera().m_vecDir.y;
    float leftY = Camera().m_vecDir.x;

    if (ILevelSystem->GetMapAt(m_vecPosition.x + leftX * moveSpeed, int(m_vecPosition.y)) == false)
      m_vecPosition.x += leftX * moveSpeed;
    if (ILevelSystem->GetMapAt(int(m_vecPosition.x), m_vecPosition.y + leftY * moveSpeed) == false)
      m_vecPosition.y += leftY * moveSpeed;
  }

  if (!IInputSystem->UseMouseMovement())
  {
    bool canPitch = IInputSystem->AllowPitch();
    if (IInputSystem->IsKeyDown(SDL_SCANCODE_UP) && canPitch)
    {
      // look up
      m_camera.m_flPitch += 400 * pitchSpeed;
      if (m_camera.m_flPitch > 200)
        m_camera.m_flPitch = 200;
    }
    if (IInputSystem->IsKeyDown(SDL_SCANCODE_DOWN) && canPitch)
    {
      // look down
      m_camera.m_flPitch -= 400 * pitchSpeed;
      if (m_camera.m_flPitch < -200)
        m_camera.m_flPitch = -200;
    }
    if (IInputSystem->IsKeyDown(SDL_SCANCODE_LEFT))
      m_camera.Rotate(rotSpeed);

    if (IInputSystem->IsKeyDown(SDL_SCANCODE_RIGHT))
      m_camera.Rotate(-rotSpeed);
  }
  else
  {
    auto mouseMove = IInputSystem->GetLastMouseMove();

    m_camera.Rotate(mouseMove.x * rotSpeed);

    if (IInputSystem->AllowPitch())
      m_camera.m_flPitch += mouseMove.y * pitchSpeed * 500;

    if (m_camera.m_flPitch > 200)
      m_camera.m_flPitch = 200;
    if (m_camera.m_flPitch < -200)
      m_camera.m_flPitch = -200;
  }

  bool isCrouching = false;
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_LCTRL))
  {
    // crouch
    isCrouching = true;
    m_vecPosition.z = -200;
  }
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_SPACE))
  {
    // jump
    // m_vecPosition.z = 200;
    
  }
  if(IInputSystem->IsMouseButtonDown(0))
  {
      GetActiveWeapon()->Shoot();
  }


  if (m_camera.m_flPitch > 0)
    m_camera.m_flPitch = std::max<double>(0, m_camera.m_flPitch - 100 * pitchSpeed);
  if (m_camera.m_flPitch < 0)
    m_camera.m_flPitch = std::min<double>(0, m_camera.m_flPitch + 100 * pitchSpeed);
  if (m_vecPosition.z > 0)
    m_vecPosition.z = std::max<double>(0, m_vecPosition.z - 100 * moveSpeed);
  if (m_vecPosition.z < 0 && !isCrouching)
    m_vecPosition.z = std::min<double>(0, m_vecPosition.z + 100 * moveSpeed);
}