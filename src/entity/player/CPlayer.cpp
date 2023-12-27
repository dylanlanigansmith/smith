#include "CPlayer.hpp"
#include <engine/engine.hpp>
#include <interfaces/IInputSystem/IInputSystem.hpp>
#include <entity/dynamic/enemy/soldier/CSoldier.hpp>
#include <light/CLightWeapon.hpp>

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

  static auto lastHealthGain = IEngineTime->GetCurLoopTick();
  if(m_health < 75 && IEngineTime->GetCurLoopTick()  > (lastHealthGain + TICKS_PER_S * 2.5 ) ){
    m_health += Util::SemiRandRange(2, 6);
    lastHealthGain = IEngineTime->GetCurLoopTick();
  }
}

void CPlayer::OnCreate()
{
  SET_ENT_NAME();
  SET_ENT_TYPE();
  
  m_health = m_maxhealth = 100;

  m_vecPosition = {19,20,0};//Vector(22, 12, 0);
  m_camera.m_vecDir = {-1, 0};
  m_camera.m_vecPlane = {0, 0.66};
  m_camera.m_flPitch = 0.0;
  

  m_camera.m_bobAmt = 6.0;
  m_scamera = m_camera;
  m_move.m_flForwardSpeed = 0.130;
  m_move.m_flStrafeSpeed = 0.1;
  m_move.m_flSpeedModifier = 1.38;
  m_move.m_flYawSpeed = 0.100;

  m_inventory = new inventory_t();

  auto Pistol = new CWeaponPistol(this);
  auto SMG = new CWeaponSMG(this);
  auto ShotGun = new CWeaponShotgun(this);
  m_inventory->AddItem( SMG);
  m_inventory->AddItem( Pistol );
  m_inventory->AddItem( ShotGun );
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
  if(IInputSystem->UseMouseMovement())
  {
    auto mouseMove = IInputSystem->GetLastMouseMove();
    //if(mouseMove.x != 0.0 || mouseMove.x != -0.0)
     // engine->log("Out %f", mouseMove.x);
    m_scamera.Rotate(mouseMove.x );

    if (IInputSystem->AllowPitch())
      m_scamera.m_flPitch += mouseMove.y * 360.f;


    static const double maxPitch = CCamera::MaxPitch();
    if (m_scamera.m_flPitch > maxPitch)
      m_scamera.m_flPitch = maxPitch;
    if (m_scamera.m_flPitch < -maxPitch)
      m_scamera.m_flPitch = -maxPitch;
  }
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

void CPlayer::OnHit(int damage, int position )
{
  m_health -= damage;
  
  //play hurt sound
  if(m_health <= 0){
   // m_move.m_flForwardSpeed = 0.0;
    //m_move.m_flStrafeSpeed = 0.0;
    engine->info("player died!");
    int er = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "You died.", "you lost the game like a little bitch", engine->window()); //No message system available -1
    if(er != 0){
      engine->warn("message boxes fucked %s", SDL_GetError());
    }

    m_health = m_maxhealth;

    
    IEntitySystem->Events()->FireEvent(this, CEventManager::EventID("change_level"), EventArg(std::string("lvldeath")), Event::OnlyProcess);
    
  }
    
}

void CPlayer::OnCollisionWith(CBaseEntity *hit)
{
  constexpr static auto enemytype = CEntitySystem::CreateType("CSoldier");
  if(hit->IsAlive() || !hit->IsEnemy()) return;
 

  auto enemy = dynamic_cast<CBaseEnemy*>(hit);
  if(!enemy->HasLoot()) return;

  int ammo = enemy->Loot();
  Inventory()->AddAmmo(ammo);
  engine->log("looted %d", ammo);
  //GetActiveWeapon()->GainAmmo(ammo);

}

void CPlayer::OnSetPosition(const Vector2 &old_pos, const Vector2 &new_pos)
{
  m_scamera.m_vecPosition = m_camera.m_vecPosition = new_pos;
}

void CPlayer::CreateMove()
{
  static bool noclip = false;



  // double frameTime = IEngineTime->GetLastFrameTime().sec() / 50.f; // ticks bro u need ticks
  static constexpr double tickTime = 1.000 / TICKS_PER_S ;
 
  double rotSpeed = m_move.m_flYawSpeed;   // the constant value is in radians/ tick
  double pitchSpeed = m_move.m_flYawSpeed; //pitch not used currently

 

  double speedMod =  1.0;
  WASD_t in_move = IInputSystem->GetInput();
  m_isMoving = false;

  if (IInputSystem->IsKeyDown(SDL_SCANCODE_LSHIFT))
  {
    // sprint
    speedMod = (noclip ) ? 2.5 : m_move.m_flSpeedModifier;
  }
   double moveSpeed = m_move.m_flForwardSpeed *  speedMod;
  double strafeSpeed = m_move.m_flStrafeSpeed *  speedMod;
  Vector velocity = {0,0,0};
  if (in_move.w)
  {
   
     if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x + Camera().m_vecDir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        velocity.x += Camera().m_vecDir.x * moveSpeed;
     if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + Camera().m_vecDir.y * moveSpeed, m_vecPosition.z}) == false)
        velocity.y += Camera().m_vecDir.y * moveSpeed;
  }
  // move backwards if no wall behind you
  if (in_move.s)
  {
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x - Camera().m_vecDir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        velocity.x -= Camera().m_vecDir.x * moveSpeed;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y - Camera().m_vecDir.y * moveSpeed, m_vecPosition.z}) == false || noclip)
        velocity.y -= Camera().m_vecDir.y * moveSpeed;
  }
  if (in_move.d)
  {
    float rightX = Camera().m_vecDir.y;
    float rightY = -Camera().m_vecDir.x;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x + rightX * strafeSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        velocity.x += rightX  * strafeSpeed;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + rightY * strafeSpeed, m_vecPosition.z}) == false || noclip)
        velocity.y += rightY  * strafeSpeed;
  }

  // move left
  if (in_move.a)
  {
    float leftX = -Camera().m_vecDir.y;
    float leftY = Camera().m_vecDir.x;

    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x + leftX  * strafeSpeed, m_vecPosition.y, m_vecPosition.z}) == false || noclip)
        velocity.x += leftX  * strafeSpeed;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + leftY  * strafeSpeed, m_vecPosition.z}) == false || noclip)
        velocity.y += leftY  * strafeSpeed;
  }


  if(velocity.LengthSqr() > moveSpeed*moveSpeed){
    //clamp
    velocity = velocity.Normalize() *  moveSpeed;

  }
   m_vecPosition.x += velocity.x;
   m_vecPosition.y += velocity.y;


  static double bob = 0;
  static auto lastStep = IEngineTime->GetCurLoopTick();
  auto curTick = IEngineTime->GetCurLoopTick();
  //https://github.com/id-Software/DOOM/blob/master/linuxdoom-1.10/p_user.c#L74-L141

  auto footsound = [](int i)
  {
    switch (i)
    {
    case 0:
      return std::string("fstepc0");
      break;
    case 1:
      return std::string("fstepc1");
      break;
    case 2:
      return std::string("fstepc2");
      break;
    case 3:
      return std::string("fstepc3");
      break;
    default:
      return std::string("fstepc0");
      break;
    }
  };
  static int step_idx = 0;
  if(step_idx > 3) step_idx = 0;
  if(velocity.LengthSqr() > 0.0005 ){
    m_isMoving = true;
    bob = m_camera.m_bobAmt * sin( (double)curTick / 1.8f);
    
    looptick_t stepTime = (speedMod > 1.01) ? TICKS_PER_S / 4.4 : TICKS_PER_S / 2.7;
    if(lastStep + stepTime < curTick){
      engine->SoundSystem()->PlaySound(footsound(step_idx), 0.4);
      step_idx++;
      lastStep = curTick;
    }
  }
  m_vecPosition.z = bob;

 
  
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
    // m_vecPosition.z -= 200;
    
  }

if (!IInputSystem->UseMouseMovement()) //arrow key look controls
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
 //moved here so that 
 if(IInputSystem->UseMouseMovement())
  {
    m_camera.m_vecDir = m_scamera.m_vecDir;
    m_camera.m_vecPlane = m_scamera.m_vecPlane;
    m_camera.m_flPitch =  m_scamera.m_flPitch;
  }
//move this to viewmodel!!!! 
  static CLightWeapon* flash = nullptr; 
  
  if(flash == nullptr)
  {
    flash = ILightingSystem->AddLight<CLightWeapon>();
    flash->SetIntensity(0.8);
    flash->SetColor(Color(185, 147, 41, 190));
    flash->SetRange(0.75);
  }
  if(velocity.LengthSqr() > 0.0001 ){
    // use cam dir to put this slightly in front of player 
    auto viewdir = m_camera.m_vecDir.Normalize() * 0.7;
    flash->SetPosition({m_vecPosition.x + viewdir.x, m_vecPosition.y + viewdir.y, 0.5f});
  }
     
  if(IInputSystem->IsMouseButtonDown(0) ) //REALLY NEED A MENU OPEN FUNCTION LIKE WTF
  {
      if(GetActiveWeapon()->Shoot()){
        flash->Flash(4);

        //recoil!! dont love this
        m_scamera.m_vecDir = m_camera.m_vecDir;
        m_scamera.m_vecPlane = m_camera.m_vecPlane;
        m_scamera.m_flPitch =  m_camera.m_flPitch;
      }
        
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
  if(IInputSystem->IsKeyDown(SDL_SCANCODE_3)){
    if(Inventory()->SlotFilled(2)){
      Inventory()->Switch(2);
    }
  }
  if(IInputSystem->IsKeyDown(SDL_SCANCODE_4)){
    if(Inventory()->SlotFilled(3)){
      Inventory()->Switch(3);
    }
  }
  if(IInputSystem->IsKeyDown(SDL_SCANCODE_5)){
    if(Inventory()->SlotFilled(4)){
      Inventory()->Switch(4);
    }
  }



    if(noclip){
       m_health = m_maxhealth;
    }
     
}