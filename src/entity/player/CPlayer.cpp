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

  m_vecPosition = {19,20,0};//Vector(22, 12, 0);
  m_camera.m_vecDir = {-1, 0};
  m_camera.m_vecPlane = {0, 0.66};
  m_camera.m_flPitch = 0.0;

  m_move.m_flForwardSpeed = 0.19;
  m_move.m_flStrafeSpeed = 0.179;
  m_move.m_flSpeedModifier = 1.33;
  m_move.m_flYawSpeed = 0.176;

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
  static bool noclip = false;
  static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
  //  static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
  // double frameTime = IEngineTime->GetLastFrameTime().sec() / 50.f; // ticks bro u need ticks
  static constexpr double tickTime = 1.000 / TICKS_PER_S ;
  double moveSpeed = m_move.m_flForwardSpeed;  // the constant value is in squares/ tick
  double strafeSpeed = m_move.m_flStrafeSpeed;
  double rotSpeed = m_move.m_flYawSpeed;   // the constant value is in radians/ tick
  double pitchSpeed = m_move.m_flYawSpeed; //pitch not used currently

  double speedMod = 1.0;
  WASD_t in_move = IInputSystem->GetInput();
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_LSHIFT))
  {
    // sprint
    speedMod = m_move.m_flSpeedModifier;
  }
  if (in_move.w)
  {
   
     if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x + Camera().m_vecDir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        m_vecPosition.x += Camera().m_vecDir.x * moveSpeed;
     if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + Camera().m_vecDir.y * moveSpeed, m_vecPosition.z}) == false)
        m_vecPosition.y += Camera().m_vecDir.y * moveSpeed;
  }
  // move backwards if no wall behind you
  if (in_move.s)
  {
    if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x - Camera().m_vecDir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        m_vecPosition.x -= Camera().m_vecDir.x * moveSpeed;
    if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x,  m_vecPosition.y - Camera().m_vecDir.y * moveSpeed, m_vecPosition.z}) == false || noclip)
        m_vecPosition.y -= Camera().m_vecDir.y * moveSpeed;
  }
  if (in_move.d)
  {
    float rightX = Camera().m_vecDir.y;
    float rightY = -Camera().m_vecDir.x;
    if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x + rightX * strafeSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        m_vecPosition.x += rightX  * strafeSpeed;
    if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + rightY * strafeSpeed, m_vecPosition.z}) == false || noclip)
        m_vecPosition.y += rightY  * strafeSpeed;
  }

  // move left
  if (in_move.a)
  {
    float leftX = -Camera().m_vecDir.y;
    float leftY = Camera().m_vecDir.x;

    if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x + leftX  * strafeSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        m_vecPosition.x += leftX  * strafeSpeed;
    if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + leftY  * strafeSpeed, m_vecPosition.z}) == false || noclip)
        m_vecPosition.y += leftY  * strafeSpeed;
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
    noclip = !noclip;
    IInputSystem->log("noclip = [%i]", noclip);
    // crouch
   // isCrouching = true;
   // m_vecPosition.z = -200;
  }
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_SPACE))
  {
    // jump
    // m_vecPosition.z = 200;
    
  }
  if(IInputSystem->IsMouseButtonDown(0) ) //REALLY NEED A MENU OPEN FUNCTION LIKE WTF
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