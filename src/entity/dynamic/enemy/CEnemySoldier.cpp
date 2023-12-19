#include "CEnemySoldier.hpp"
#include <engine/engine.hpp>
#define HEAD_FIX 0.35f

REGISTER_DEF_ENT(CEnemySoldier);

void CEnemySoldier::OnUpdate()
{
    static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
     static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");

     auto curTick = IEngineTime->GetCurLoopTick();
    if(m_state == Dying || m_state == Dead)
    {
        m_blocking = false;
        m_bounds = 0.45;
        if(m_isDying == false){
            m_nextBehaviourChange = curTick + 21;
            m_isDying = true;
            m_state = Dying;
        }
            
        if(m_state == Dying && curTick >  m_nextBehaviourChange){
             m_state = Dead;
             draw.params.vOffset *= 2.5;
        }      
        m_anim.OnUpdate();
        return;
    }
    auto player = IEntitySystem->GetLocalPlayer();
    auto player_pos = player->GetPosition();
    m_state = Default; //must be overwritten

    if( curTick >=m_nextBehaviourChange && m_nextBehaviour != -1){
        m_behaviour = m_nextBehaviour;
        m_nextBehaviourChange = 0;
        m_nextBehaviour = -1;
    }
    if(m_behaviour == Behaviour_Default){
        if(m_state != Aiming)
            m_state = Standing;
       // m_anim.log("waiting for %s now%li then%li", magic_enum::enum_name((CEnemySoldier::SoldierBehaviour)m_nextBehaviour).data(), curTick, m_nextBehaviourChange);
        m_anim.OnUpdate();
        return;
    }
    if(m_behaviour == Behaviour_Patrol)
    {
        if(!m_path.HasPath()){
            Vector2 empty = ILevelSystem->FindEmptySpace();
            for(;;)
            {
                 
                empty = { empty.x + 0.4, empty.y + 0.4};
                if( ( Vector2(m_vecPosition) - empty).LengthSqr()  > 6 )
                    break;
                empty = ILevelSystem->FindEmptySpace();
            }
           
           // empty = player_pos;
            if(m_path.Search({m_vecPosition.x, m_vecPosition.y},  empty))
            {
                 m_headingTo = m_path.GetNextPoint(Vector2(m_vecPosition));  m_headingTo = { m_headingTo.x + HEAD_FIX, m_headingTo.y + HEAD_FIX};
               // m_path.log("Found Path! goin to {%.3f ,%.3f} im at {%.3f ,%.3f}", empty.x,empty.y, m_vecPosition.x, m_vecPosition.y);
            }
            else{
               // m_path.log("no path to {%.3f ,%.3f} from {%.3f ,%.3f}", empty.x,empty.y , m_vecPosition.x, m_vecPosition.y);
            }
           
        }
        if(m_path.HasPath())
        {
            if( ILevelSystem->GetTileAt(m_headingTo)->m_nType == Level::Tile_Wall || m_path.ReachedGoal()  )
            {
               // m_path.log("yeah");
                m_view.lookAt(m_vecPosition, Vector2(m_headingTo.x, m_headingTo.y));
                m_path.Reset();
                m_behaviour = Behaviour_Default;
                m_nextBehaviour = Behaviour_Patrol;
                m_nextBehaviourChange = curTick + TICKS_PER_S;
            }
            else{
                 
                
               
                WalkTowards({m_headingTo.x, m_headingTo.y });
            }
             if(Vector2::closeEnough(m_vecPosition, m_headingTo, 0.1)){
                  //  m_path.log("got to {%.3f ,%.3f} im at {%.3f ,%.3f}", m_headingTo.x, m_headingTo.y, m_vecPosition.x, m_vecPosition.y);
                    m_headingTo = m_path.GetNextPoint(Vector2(m_headingTo));  m_headingTo = { m_headingTo.x + HEAD_FIX, m_headingTo.y + HEAD_FIX};
                  //  m_path.log("now im goin to {%.3f ,%.3f} im at {%.3f ,%.3f}  goal {%.3f ,%.3f}", m_headingTo.x, m_headingTo.y, m_vecPosition.x, m_vecPosition.y, m_path.GetGoal().x, m_path.GetGoal().y);
                    
                }
            
        }
        if(isPlayerVisible(player, 45.0))
        {
          //  m_anim.log("HEY! %li", curTick);
            
            //play sound HEY
             m_view.lookAt(m_vecPosition, player_pos);
            m_path.Reset();
            m_path.Search({m_vecPosition.x, m_vecPosition.y}, {player_pos.x, player_pos.y});
            if(m_path.HasPath()){
                //m_behaviour =  Behaviour_Reposition;
                m_state = Default;
                if(m_lastShout < curTick){
                    engine->SoundSystem()->PlayPositional ("soldier_hey", m_vecPosition);
                    m_lastShout = curTick + TICKS_PER_S * 4;
                }
                 
                m_nextBehaviour = Behaviour_Reposition;
                m_nextBehaviourChange = curTick + 2;

                 m_headingTo = m_path.GetNextPoint(Vector2(m_vecPosition));  m_headingTo = { m_headingTo.x + HEAD_FIX, m_headingTo.y + HEAD_FIX};
            }
            else{

                m_behaviour = Behaviour_Default;
                m_state = Aiming;
                m_nextBehaviour = Behaviour_Aiming;
                m_nextBehaviourChange = curTick + TICKS_PER_S / 3;
            }
        }
    }
    if(m_behaviour == Behaviour_Reposition)
    {
        
        if(!isPlayerVisible(player, 50.0) && !m_path.HasPath())//wider fov
        {
            m_path.Reset();
            m_path.Search({m_vecPosition.x, m_vecPosition.y}, {player_pos.x, player_pos.y});
           //path will still be to last spotted pt??
            m_view.lookAt(m_vecPosition, player_pos);
            m_behaviour = Behaviour_Default;
            m_nextBehaviour = Behaviour_Patrol;
            m_nextBehaviourChange = curTick + TICKS_PER_S / 4;
            m_anim.dbg("wtf we lost em");
        }
    
            if(m_path.HasPath()){
                auto nextPoint = m_path.GetNextPoint(IVector2(m_vecPosition));
                double oldMove = m_move.m_flForwardSpeed;
                m_move.m_flForwardSpeed *= m_move.m_flSpeedModifier;
                WalkTowards({nextPoint.x + 0.3, nextPoint.y + 0.3});

                m_move.m_flForwardSpeed = oldMove;
            }
            else{
                 m_view.lookAt(m_vecPosition, player_pos);
                 m_path.Search({m_vecPosition.x, m_vecPosition.y}, {player_pos.x, player_pos.y});
            }
            if( (player_pos - m_vecPosition).Length2D() < 8){ //arbitrary
                     m_view.lookAt(m_vecPosition, player_pos, m_move.m_flYawSpeed);
                    m_behaviour = Behaviour_Default;
                    m_nextBehaviour = Behaviour_Aiming;
                    m_state = Aiming;
                    m_nextBehaviourChange = curTick + 1;
                   //   m_anim.log("we gonna aim");
            }
    }
    
    if(m_behaviour == Behaviour_Aiming){
        //aiming animation
        m_state = Aiming;
        if(m_nextBehaviour != Behaviour_Attack)
        {
             m_view.lookAt(m_vecPosition, player_pos);
           // m_behaviour = Behaviour_Default;
            m_nextBehaviour = Behaviour_Attack;
            m_nextBehaviourChange = curTick + TICKS_PER_S / 3;
            if(!isPlayerVisible(player, 40.0))//wider fov
            {
                
            //path will still be to last spotted pt??
                m_view.lookAt(m_vecPosition, player_pos);
                m_behaviour = Behaviour_Default;
                m_nextBehaviour = Behaviour_Patrol;
                m_nextBehaviourChange = curTick + TICKS_PER_S / 4;
                
            }
        }
    }
    if(m_behaviour == Behaviour_Attack){
        m_state = Attacking;
        if(m_nextBehaviour != Behaviour_PostAttack)
        {
            
             Shoot(player);
            m_view.lookAt(m_vecPosition, player_pos, m_move.m_flYawSpeed);
            //playsound gun 
            m_nextBehaviour = Behaviour_PostAttack;
            m_nextBehaviourChange = curTick + TICKS_PER_S / 3;
        
        }
    }
     if(m_behaviour == Behaviour_PostAttack){
         m_state = Attacking;
  
       
        
        if(Util::SemiRandRange(0, 100) > 75){
            m_nextBehaviour = Behaviour_Aiming;
             m_nextBehaviourChange = curTick;
            m_behaviour = Behaviour_Aiming;
                    m_state = Aiming;   
            m_anim.log("lucky");
        }
        else{
            m_nextBehaviour = Behaviour_Reposition;
            m_nextBehaviourChange = curTick + 8;
            m_path.Reset();
            m_path.Search({m_vecPosition.x, m_vecPosition.y}, {player_pos.x, player_pos.y});
            m_anim.log("unlucky");
        }
    }
       

    m_anim.OnUpdate();

}

void CEnemySoldier::OnCreate()
{
    ENT_SETUP();

    m_state = Default;
    m_behaviour = Behaviour_Patrol;
    m_lastShout = 0;
    m_nextBehaviourChange = 0;
    m_nextBehaviour = -1; //these should be AI states not anim states
    m_move.m_flForwardSpeed = 0.03; //0.130
    m_move.m_flStrafeSpeed = 0.1;
    m_move.m_flSpeedModifier = 1.38;
    m_move.m_flYawSpeed = 0.2;
    m_bounds = 0.35;
    m_hasDroppedLoot = false;
    m_blocking = true;
    m_stats.m_main_damage = 8; 
    m_stats.m_alt_damage = 4;
    m_isDying = false;
    m_anim.OnCreate();
    CreateRenderable();
}
void CEnemySoldier::WalkTowards(const Vector2& pos)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    double moveSpeed = m_move.m_flForwardSpeed;
    if(!m_view.lookAt(m_vecPosition, pos, m_move.m_flYawSpeed))
        moveSpeed /= 2.0;
    
    m_state = Walking;
    Vector newPos = m_vecPosition;
    bool wasCol = false;
    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x + m_view.m_dir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false )
        newPos.x += m_view.m_dir.x * moveSpeed;
    else wasCol = true;

    if(ILevelSystem->IsCollision(this, m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + m_view.m_dir.y * moveSpeed, m_vecPosition.z}) == false)
        newPos.y += m_view.m_dir.y * moveSpeed;
    else wasCol = true;
    
    SetPosition(newPos);
    if( ( m_vecPosition.x != newPos.x || m_vecPosition.y != newPos.y  || wasCol ) && m_path.HasPath())
        m_path.Reset();

}


//just keep this function minimized okay? you can thank me later. god help you. 
int CEnemySoldier::DeduceSequenceForOrientation(int *flip, int *anim_state, int *frame, std::string &seq_name) // returns orientation
{
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    auto player = IEntitySystem->GetLocalPlayer();
    //set defaults in case we fail along the way
    const std::string def_seq_name = "stand0";
    *flip = AnimDir_NoFlip; *anim_state = AnimDir_Default; seq_name = def_seq_name; *frame = -1;
    int orient = CAnimDirectional::DefaultOrientation;

    //Calc Angle To Player
    Vector2 delta = m_vecPosition - player->GetPosition();
    delta = delta.Normalize();
    Vector2 viewDir = m_view.m_dir.Normalize();

    double angle = std::acos(viewDir.dotClamped(delta)) * RAD2DEGNUM; //frontal angle: // towards->0 = behind // towards->180 = infront

    double cross = viewDir.cross(delta); //neg = their left (based on facing dir?)
    
  //  engine->dbg("ang (0-180) %f | crs (+-) %f", angle, cross);

    bool facing = angle > 90;

    const double tol = 30.0;
    const double side_tol = 30.0 ; //90 +- sidetol
    const double xtol = 0.1;
    switch (m_state)
    {
    case Dying:
    {
       
        orient = CAnimDirectional::Face_Dying;
        *flip = AnimDir_NoFlip;
        *anim_state = AnimDir_Dying; 
        if(m_iID % 2 == 0)
            seq_name = "death0";
        else
            seq_name = "death1";
            
        break;
        
    }
    case Dead:
    {
       
        orient = CAnimDirectional::Dead;
        *flip = AnimDir_NoFlip;
        *anim_state = AnimDir_Dead; 
        seq_name = "dead0"; break;
        
    }
    case Attacking:
    {
        if(!facing && angle < tol){
            orient = CAnimDirectional::Away;
             *flip = AnimDir_NoFlip;
            *anim_state = AnimDir_Attacking; 
            seq_name = "shoot4"; break;
        }
        if(facing && angle > (180 - tol) ){
            orient = CAnimDirectional::Facing;
             *flip = AnimDir_NoFlip;
            *anim_state = AnimDir_Attacking; 
            seq_name = "shoot0"; break;
        }
        if( (90 - side_tol) < angle && angle < (90 + side_tol) ){
                 orient =  (cross >= xtol) ? CAnimDirectional::Face_Right : CAnimDirectional::Face_Left;
                *flip = (cross >= xtol) ?  AnimDir_FlipH : AnimDir_NoFlip;
                *anim_state = AnimDir_Attacking; 
                seq_name = "shoot2"; break;
        }
        if(!facing &&  tol < angle){
            
            if(cross >= xtol){
                orient = CAnimDirectional::Away_DiagLeft;
                *flip = AnimDir_FlipH;
                *anim_state = AnimDir_Attacking; 
                seq_name = "shoot3"; break;
            }
            else if(cross <= -xtol){
                orient = CAnimDirectional::Away_DiagRight; //no idea
                 *flip = AnimDir_NoFlip;
                *anim_state = AnimDir_Attacking; 
                seq_name = "shoot3"; break;
            }
           
        }
        
        if(facing &&  angle < (180.0 - tol)){
            if(cross >= xtol){
                orient = CAnimDirectional::Face_DiagLeft;
                *flip = AnimDir_FlipH;
                *anim_state = AnimDir_Attacking; 
                seq_name = "shoot1"; break;
            }
            else if(cross <= -xtol){
                orient = CAnimDirectional::Face_DiagRight; //idk
                 *flip = AnimDir_NoFlip;
                *anim_state = AnimDir_Attacking; 
                seq_name = "shoot1"; break;
            }
           
        }
        engine->log(" shoot hit no cases  <%.2f | x%.2f", angle, cross);
        break;
    }
    case Aiming:
    {
        if(!facing && angle < tol){
            orient = CAnimDirectional::Away;
            *frame = 0 ;
             *flip = AnimDir_NoFlip;
            *anim_state = AnimDir_Attacking; 
            seq_name = "shoot4"; break;
        }
        if(facing && angle > (180 - tol) ){
            orient = CAnimDirectional::Facing;
             *flip = AnimDir_NoFlip; *frame = 0 ;
            *anim_state = AnimDir_Attacking; 
            seq_name = "shoot0"; break;
        }
        if( (90 - side_tol) < angle && angle < (90 + side_tol) ){
                 orient =  (cross >= xtol) ? CAnimDirectional::Face_Right : CAnimDirectional::Face_Left;
                *flip = (cross >= xtol) ?  AnimDir_FlipH : AnimDir_NoFlip;
                *anim_state = AnimDir_Attacking; *frame = 0 ;
                seq_name = "shoot2"; break;
        }
        if(!facing &&  tol < angle){
            
            if(cross >= xtol){
                orient = CAnimDirectional::Away_DiagLeft;
                *flip = AnimDir_FlipH;
                *anim_state = AnimDir_Attacking; *frame = 0 ;
                seq_name = "shoot3"; break;
            }
            else if(cross <= -xtol){
                orient = CAnimDirectional::Away_DiagRight; //no idea
                 *flip = AnimDir_NoFlip;
                *anim_state = AnimDir_Attacking; *frame = 0 ;
                seq_name = "shoot3"; break;
            }
           
        }
        
        if(facing &&  angle < (180.0 - tol)){
            if(cross >= xtol){
                orient = CAnimDirectional::Face_DiagLeft;
                *flip = AnimDir_FlipH;
                *anim_state = AnimDir_Attacking; *frame = 0 ;
                seq_name = "shoot1"; break;
            }
            else if(cross <= -xtol){
                orient = CAnimDirectional::Face_DiagRight; //idk
                 *flip = AnimDir_NoFlip;
                *anim_state = AnimDir_Attacking; *frame = 0 ;
                seq_name = "shoot1"; break;
            }
           
        }
        engine->log(" aim hit no cases  <%.2f | x%.2f", angle, cross);
        break;
    }
    case Walking:
    {
        if(!facing && angle < tol){
            orient = CAnimDirectional::Away;
             *flip = AnimDir_NoFlip;
            *anim_state = AnimDir_Walking; 
            seq_name = "walkaway0"; break;
        }
        if(facing && angle > (180 - tol) ){
            orient = CAnimDirectional::Facing;
             *flip = AnimDir_NoFlip;
            *anim_state = AnimDir_Walking; 
            seq_name = "walkfwd0"; break;
        }
        if( (90 - side_tol) < angle && angle < (90 + side_tol) ){
                 orient =  (cross >= xtol) ? CAnimDirectional::Face_Right : CAnimDirectional::Face_Left;
                *flip = (cross >= xtol) ?  AnimDir_FlipH : AnimDir_NoFlip;
                *anim_state = AnimDir_Walking; 
                seq_name = "walkside0"; break;
        }
        if(!facing &&  tol < angle){
            
            if(cross >= xtol){
                orient = CAnimDirectional::Away_DiagLeft;
                *flip = AnimDir_FlipH;
                *anim_state = AnimDir_Walking; 
                seq_name = "walkdiag1"; break;
            }
            else if(cross <= -xtol){
                orient = CAnimDirectional::Away_DiagRight; //no idea
                 *flip = AnimDir_NoFlip;
                *anim_state = AnimDir_Walking;
                seq_name = "walkdiag1"; break;
            }
           
        }
        
        if(facing &&  angle < (180.0 - tol)){
            if(cross >= xtol){
                orient = CAnimDirectional::Face_DiagLeft;
                *flip = AnimDir_FlipH;
                *anim_state = AnimDir_Walking; 
                seq_name = "walkdiag0"; break;
            }
            else if(cross <= -xtol){
                orient = CAnimDirectional::Face_DiagRight; //idk
                 *flip = AnimDir_NoFlip;
                *anim_state = AnimDir_Walking; 
                seq_name = "walkdiag0"; break;
            }
           
        }
        engine->log(" walk hit no cases  <%.2f | x%.2f {%.3f %.3f}", angle, cross, m_view.m_dir.x, m_view.m_dir.y);
        break;
    }
    case Standing:
    default:
    {
        if(!facing && angle < tol){
            orient = CAnimDirectional::Away;
            *frame = 4; *flip = AnimDir_NoFlip;
            *anim_state = AnimDir_Standing; 
            seq_name = "stand0"; break;
        }
        if(facing && angle > (180 - tol) ){
            orient = CAnimDirectional::Facing;
            *frame = 0; *flip = AnimDir_NoFlip;
            *anim_state = AnimDir_Standing; 
            seq_name = "stand0"; break;
        }
        if( (90 - side_tol) < angle && angle < (90 + side_tol) ){
                 orient =  (cross >= xtol) ? CAnimDirectional::Face_Right : CAnimDirectional::Face_Left;
                 *frame = 2; *flip = (cross >= xtol) ?  AnimDir_FlipH : AnimDir_NoFlip;
                *anim_state = AnimDir_Standing; 
                seq_name = "stand0"; break;
        }
        if(!facing &&  tol < angle){
            
            if(cross >= xtol){
                orient = CAnimDirectional::Away_DiagLeft;
                 *frame = 3; *flip = AnimDir_FlipH;
                *anim_state = AnimDir_Standing; 
                seq_name = "stand0"; break;
            }
            else if(cross <= -xtol){
                orient = CAnimDirectional::Away_DiagRight; //no idea
                 *frame = 3; *flip = AnimDir_NoFlip;
                *anim_state = AnimDir_Standing;
                seq_name = "stand0"; break;
            }
           
        }
        
        if(facing &&  angle < (180.0 - tol)){
            if(cross >= xtol){
                orient = CAnimDirectional::Face_DiagLeft;
                 *frame = 1; *flip = AnimDir_FlipH;
                *anim_state = AnimDir_Standing; 
                seq_name = "stand0"; break;
            }
            else if(cross <= -xtol){
                orient = CAnimDirectional::Face_DiagRight; //idk
                 *frame = 1; *flip = AnimDir_NoFlip;
                *anim_state = AnimDir_Standing; 
                seq_name = "stand0"; break;
            }
           
        }
        engine->log(" stand hit no cases  <%.2f | x%.2f", angle, cross);
        break;
    }
        
    }
    static int lastOrient = 0;
    if(lastOrient != orient){
         //engine->log("facing %s <%.2f | x%.2f", magic_enum::enum_name((CAnimDirectional::AnimOrientations(orient))).data(), angle, cross);

    }
    lastOrient = orient;
    return orient;
}

bool CEnemySoldier::isPlayerVisible(CPlayer *player, double fov)
{
    static auto IInputSystem = engine->CreateInterface<CInputSystem>("IInputSystem");
    static bool ignore_player = PLATFORM.LaunchOptions().HasArg("peace");
    if(ignore_player || IInputSystem->isDevMenuOpen()) return false;
    #ifdef IGNORE_PLAYER
    return false;
    #endif

    if(isPlayerWithinFOV(player->GetPosition(), fov))
    {
        if(CastRayToPlayer(player->GetPosition(), player->GetBounds()))
            return true;
    }
    return false;
}

bool CEnemySoldier::isPlayerWithinFOV(const Vector2 &playerPosition, double FOVAngleDegrees)
{
      Vector2 delta = playerPosition - Vector2(GetPosition());
 
    double dot = m_view.m_dir.Normalize().dot(delta.Normalize());

    double halfFOVRadians = (FOVAngleDegrees / 2.0) * (M_PI / 180.0);
    double cosHalfFOV = cos(halfFOVRadians);

    return dot >= cosHalfFOV;

}

bool CEnemySoldier::CastRayToPlayer(const Vector2 &playerPosition, float playerBounds)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
     Ray_t ray = {
        .origin = Vector2(GetPosition()),
        .direction = (playerPosition - GetPosition()).Normalize() 
    };
    if(Util::RayIntersectsCircle(ray, playerPosition, playerBounds))
    {
        //engine->log("in circle");
        int hit = 0;
        Vector2 ray_pos = ray.origin;
        double step = 0.2;
        double dist_to_player = (playerPosition - ray.origin).Length();
        IVector2 lastPos = {-1,-1};
        while((playerPosition- ray_pos).Length() >= playerBounds)
        {
            //engine->log("CastRay %.3f %.3f", ray_pos.x, ray_pos.y);
            ray_pos = ray_pos + (ray.direction * step);
          
            if(ray_pos.x < 0 || ray_pos.y < 0) return false;
            if((int)floor(ray_pos.x) == lastPos.x && (int)floor(ray_pos.y) == lastPos.y)
                continue;
            lastPos = ray_pos;
            auto tile = ILevelSystem->GetTileAt(lastPos);
            if(tile->isEmpty()) continue;
            if(tile->m_nType == Level::Tile_Wall || (tile->IsThinWall() && tile->m_pTexture->isTransparent()) )
                return false;
            
        }
        return true;
        //engine->log("cast ray hit player");
    }

    return false;

}

int CEnemySoldier::Loot()
{
     if(m_state != Dead) 
        return 0; 
    m_hasDroppedLoot = true; 
    return Util::SemiRandRange(10, 30); 
}

void CEnemySoldier::Shoot(CPlayer *player)
{

   
    //ray cast??
    //how do we make it "fair"
    auto player_pos = player->GetPosition();
     engine->SoundSystem()->PlayPositional ("mp5", m_vecPosition, 0.1, 0.6);
    float player_bounds = player->GetBounds();
    Ray_t ray = {
        .origin = Vector2(GetPosition()),
        .direction = m_view.m_dir.Normalize() //should already be
    };
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    if(Util::RayIntersectsCircle(ray, player_pos, player_bounds)) //this is literally CastRayToPlayer...
    {
        //engine->log("in circle");
        int hit = 0;
        Vector2 ray_pos = ray.origin;
        double step = 0.2;
        double dist_to_player = (Vector2(player_pos) - ray.origin).Length();
        IVector2 lastPos = {-1,-1};
        while((Vector2(player_pos) - ray_pos).Length() >= player_bounds)
        {
           //engine->log("%.3f %.3f", ray_pos.x, ray_pos.y);
            ray_pos = ray_pos + (ray.direction * step);
          
            if(ray_pos.x < 0 || ray_pos.y < 0) return;
            if((int)floor(ray_pos.x) == lastPos.x && (int)floor(ray_pos.y) == lastPos.y)
                continue;
            lastPos = ray_pos;
            auto tile = ILevelSystem->GetTileAt(lastPos);
            if(tile->Blocking())
                return;
            
        }
        player->OnHit(m_stats.m_main_damage + (Util::SemiRandRange(0, m_stats.m_alt_damage) - m_stats.m_alt_damage ));
       // engine->log("hit player");
    }
}


uint32_t CEnemySoldier::GetPixelAtPoint(CCamera *camera, const IVector2 &point, IVector2 *textpos)
{
    draw.camera = camera;
    CalculateDrawInfo(draw);
    return m_anim.GetPixelAtPoint(point, textpos, draw);
    
}
void CEnemySoldier::CreateRenderable()
{
    auto seq_stand = m_anim.AddDefaultSequenceByName("stand0", {64, 64});
    m_anim.AddSequenceByName("walkside0");
    m_anim.AddSequenceByName("walkfwd0");
    m_anim.AddSequenceByName("walkaway0");
    m_anim.AddSequenceByName("walkdiag0");
    m_anim.AddSequenceByName("walkdiag1");

    m_anim.AddSequenceByName("shoot0"); //set x to match if you dont want shaking
    m_anim.AddSequenceByName("shoot1");
    m_anim.AddSequenceByName("shoot2");
    m_anim.AddSequenceByName("shoot3");
    m_anim.AddSequenceByName("shoot4");

    m_anim.AddSequenceByName("death0");
    m_anim.AddSequenceByName("death1");
    m_anim.AddSequenceByName("dead0");
    m_Texture = new texture_t(seq_stand->GetTexture()->m_handle, m_anim.Drawable());
    draw.params.wScale = 1.15; 
    draw.params.vScale = 1.15;
    draw.params.vOffset = 85;

    m_anim.SetCallback(std::bind(&CEnemySoldier::DeduceSequenceForOrientation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    m_view.m_dir = {-1, 0};
}



void CEnemySoldier::Render(CRenderer *renderer)
{
    draw.camera = renderer->GetActiveCamera();
    CalculateDrawInfo(draw);
    m_anim.Draw(renderer, draw);
}
