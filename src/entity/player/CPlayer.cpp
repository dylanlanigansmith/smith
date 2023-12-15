#include "CPlayer.hpp"
#include <engine/engine.hpp>
#include <interfaces/IInputSystem/IInputSystem.hpp>
#include <entity/dynamic/enemy/CEnemySoldier.hpp>
CPlayer::CPlayer(int m_iID) : CBaseRenderable(m_iID), m_viewmodel(this)
{
}

CPlayer::~CPlayer()
{
}

void CPlayer::OnUpdate()
{
  m_inventory->OnUpdate();
  CreateMove();
  static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
  static auto lastHealthGain = IEngineTime->GetCurLoopTick();
  if(m_health < 75 && IEngineTime->GetCurLoopTick()  > (lastHealthGain + TICKS_PER_S * 2.5 ) ){
    m_health += Util::SemiRandRange(2, 8);
    lastHealthGain = IEngineTime->GetCurLoopTick();
  }
}

void CPlayer::OnCreate()
{
  SET_ENT_NAME();
  SET_ENT_TYPE();
  
  m_health = m_max_health = 100;

  m_vecPosition = {19,20,0};//Vector(22, 12, 0);
  m_camera.m_vecDir = {-1, 0};
  m_camera.m_vecPlane = {0, 0.66};
  m_camera.m_flPitch = 0.0;

  m_move.m_flForwardSpeed = 0.130;
  m_move.m_flStrafeSpeed = 0.1;
  m_move.m_flSpeedModifier = 1.38;
  m_move.m_flYawSpeed = 0.100;

  m_inventory = new inventory_t();

  auto Pistol = new CWeaponPistol(this);
  auto SMG = new CWeaponSMG(this);
  m_inventory->AddItem( SMG);
  m_inventory->AddItem( Pistol );
  //MOVE THIS
  for(auto& item : m_inventory->contents()){
    if(item == nullptr) continue;
    item->SetOwnerEntity(m_iID);
    item->OnCreate();
  }
  m_viewmodel.Setup(m_inventory);
  m_viewmodel.Settings().m_crosshairColor = Color::White();
  m_viewmodel.Settings().m_crosshairWidth = 0;
  m_viewmodel.Settings().m_crosshairLength = 2;
  m_viewmodel.Settings().m_crosshairDot = false;
  m_viewmodel.Settings().m_crosshairAltColor = Color::Black();
}

void CPlayer::OnDestroy()
{
  delete m_inventory;
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
   // GetActiveWeapon()->Render(renderer);

}

void CPlayer::RenderView(CRenderer *renderer)
{
  m_viewmodel.Render(renderer);
}

void CPlayer::OnHit(int damage)
{
  m_health -= damage;
  engine->log("player health: %d", m_health);
  //play hurt sound
  if(m_health <= 0){
    m_move.m_flForwardSpeed = 0.0;
    m_move.m_flStrafeSpeed = 0.0;
    int er = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "You died.", "you lost the game like a little bitch", engine->window()); //No message system available -1
    
  }
    
}

void CPlayer::OnCollisionWith(CBaseEntity *hit)
{
  constexpr static auto enemytype = CEntitySystem::CreateType("CEnemySoldier");
  if(hit->GetType() != enemytype) return;

  auto enemy = (CEnemySoldier*)hit;

  if(enemy->GetHealth() > 0) return;

  if(!enemy->HasLoot()) return;

  int ammo = enemy->Loot();
  GetActiveWeapon()->GainAmmo(ammo);

}

void CPlayer::CreateMove()
{
  static bool noclip = false;
  static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
  //  static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
  static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
  // double frameTime = IEngineTime->GetLastFrameTime().sec() / 50.f; // ticks bro u need ticks
  static constexpr double tickTime = 1.000 / TICKS_PER_S ;
 
  double rotSpeed = m_move.m_flYawSpeed;   // the constant value is in radians/ tick
  double pitchSpeed = m_move.m_flYawSpeed; //pitch not used currently

  static bool viewPunch = false;
  static int bob = 0;
  const int maxbob = 11;
  static bool up = true;
  static bool shutOffNextZero = false;
  if(viewPunch && false)
  {
      
    if(bob >= maxbob){
      up = false;
      shutOffNextZero = true;
    } 
    if(bob <= -maxbob ){
      up = true;
      
    } 
    if(shutOffNextZero && bob >= 0){
      viewPunch = false;
      shutOffNextZero = false;
      up = true;
      bob = 0;
    }
    if(up) bob += 4;
    else bob -= 4;
  }
  m_camera.m_flPitch = bob;

  double speedMod = (noclip ) ? 1.5 : 1.0;
  WASD_t in_move = IInputSystem->GetInput();
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_LSHIFT))
  {
    // sprint
    speedMod = m_move.m_flSpeedModifier;
  }
   double moveSpeed = m_move.m_flForwardSpeed *  speedMod;
  double strafeSpeed = m_move.m_flStrafeSpeed *  speedMod;

  if (in_move.w)
  {
   
     if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x + Camera().m_vecDir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        m_vecPosition.x += Camera().m_vecDir.x * moveSpeed;
     if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + Camera().m_vecDir.y * moveSpeed, m_vecPosition.z}) == false)
        m_vecPosition.y += Camera().m_vecDir.y * moveSpeed;
  }
  // move backwards if no wall behind you
  if (in_move.s)
  {
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x - Camera().m_vecDir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        m_vecPosition.x -= Camera().m_vecDir.x * moveSpeed;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y - Camera().m_vecDir.y * moveSpeed, m_vecPosition.z}) == false || noclip)
        m_vecPosition.y -= Camera().m_vecDir.y * moveSpeed;
  }
  if (in_move.d)
  {
    float rightX = Camera().m_vecDir.y;
    float rightY = -Camera().m_vecDir.x;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x + rightX * strafeSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        m_vecPosition.x += rightX  * strafeSpeed;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + rightY * strafeSpeed, m_vecPosition.z}) == false || noclip)
        m_vecPosition.y += rightY  * strafeSpeed;
  }

  // move left
  if (in_move.a)
  {
    float leftX = -Camera().m_vecDir.y;
    float leftY = Camera().m_vecDir.x;

    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x + leftX  * strafeSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        m_vecPosition.x += leftX  * strafeSpeed;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + leftY  * strafeSpeed, m_vecPosition.z}) == false || noclip)
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
    //if(mouseMove.x != 0.0 || mouseMove.x != -0.0)
     // engine->log("Out %f", mouseMove.x);
    m_camera.Rotate(mouseMove.x );

    if (IInputSystem->AllowPitch())
      m_camera.m_flPitch += mouseMove.y * pitchSpeed * 500;

    if (m_camera.m_flPitch > 200)
      m_camera.m_flPitch = 200;
    if (m_camera.m_flPitch < -200)
      m_camera.m_flPitch = -200;
  }

  bool isCrouching = false;
  static bool downLast = false;
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_LCTRL))
  {
    if(!downLast){
       noclip = !noclip;
       gLog("set noclip -> [%i]", noclip);
       
     
    }
      
    downLast = true;
   
    // crouch
   // isCrouching = true;
   // m_vecPosition.z = -200;
  }
  else downLast = false;
  if (IInputSystem->IsKeyDown(SDL_SCANCODE_SPACE))
  {
    // jump
    // m_vecPosition.z = 200;
    
  }
  if(IInputSystem->IsMouseButtonDown(0) ) //REALLY NEED A MENU OPEN FUNCTION LIKE WTF
  {
      if(GetActiveWeapon()->Shoot()) viewPunch = true; 
  }
  if(IInputSystem->IsKeyDown(SDL_SCANCODE_R)){
    GetActiveWeapon()->Reload();
  }
   if(IInputSystem->IsKeyDown(SDL_SCANCODE_1)){
    if(Inventory()->SlotFilled(0)){
      Inventory()->Switch(0);
    }
  }
   if(IInputSystem->IsKeyDown(SDL_SCANCODE_2)){
    if(Inventory()->SlotFilled(1)){
      Inventory()->Switch(1);
    }
  }
  if (m_camera.m_flPitch > 0)
    m_camera.m_flPitch = std::max<double>(0, m_camera.m_flPitch - 100 * pitchSpeed);
  if (m_camera.m_flPitch < 0)
    m_camera.m_flPitch = std::min<double>(0, m_camera.m_flPitch + 100 * pitchSpeed);
  if (m_vecPosition.z > 0)
    m_vecPosition.z = std::max<double>(0, m_vecPosition.z - 100 * moveSpeed);
  if (m_vecPosition.z < 0 && !isCrouching)
    m_vecPosition.z = std::min<double>(0, m_vecPosition.z + 100 * moveSpeed);


    if(noclip)
      m_health = m_max_health;
}