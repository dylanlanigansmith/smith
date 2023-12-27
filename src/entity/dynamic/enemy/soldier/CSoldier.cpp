#include "CSoldier.hpp"
#include <engine/engine.hpp>
#include "../enemy_util.hpp"
REGISTER_DEF_ENT(CSoldier);

void CSoldier::OnUpdate()
{
    m_behave.OnUpdate();
    m_anim.OnUpdate();
}

void CSoldier::OnCreate()
{
    ENT_SETUP();
    
   m_lastVocal = m_lastFootstep = 0;
     
    /*
    Needs to be set:
    health/maxhealth
    team
    loot
    combat

    move 
    bounds
    view

    action
    anim /create renderable

    */
    m_loot.m_used = false;
    m_loot.m_type = 0;
    m_loot.m_amount = {10, 25};

    m_combat.m_accuracy = 0.99;
    
    m_combat.m_damage_primary = 8;
    m_combat.m_damage_variable = 5;
    m_combat.m_damage_alt = 0;

    m_health = m_maxhealth = 50;
    m_team = Team_Enemy;
    m_move.m_flForwardSpeed = 0.05; //0.130
    m_move.m_flStrafeSpeed = 0.1;
    m_move.m_flSpeedModifier = 1.38;
    m_move.m_flYawSpeed = 0.2;
    m_bounds = 0.35;

    m_action = Default;
    m_anim.OnCreate();
    CreateRenderable();
    CreateBehaviours();
}
void CSoldier::CreateRenderable()
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
    m_Texture = new texture_t(seq_stand->GetTexture()->m_handle, m_anim.Drawable()); //hack for editor
    

    m_anim.SetCallback(std::bind(&CSoldier::DeduceSequenceForOrientation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    draw.params.wScale = 1.15; 
    draw.params.vScale = 1.15;
    draw.params.vOffset = SCREEN_HEIGHT * 0.2355; //@ 360p = 85  @ 540 = 127 
    m_view.m_dir = {-1, 0};
}
void CSoldier::CreateBehaviours()
{
    behaviourFn def = [this](looptick_t start, looptick_t current){
        this->m_action = Standing;
    };
    m_behave.AddBehaviour("default", {def}, true);
    behaviourFn death = [this](looptick_t start, looptick_t current){
        this->m_blockingCollisions = false;
        this->m_bounds = 0.45;
        if(current < start + 20){ //length of death animation
            this->m_action = Dying;
        }
        else{
            if(this->m_action != Dead)
                this->draw.params.vOffset *= 2.5; //at 360p *= 2.5
            this->m_action = Dead;

        }
    };
    m_behave.AddBehaviour("death", {death});

    m_behave.AddBehaviour("patrol", {
        [this](looptick_t start, looptick_t current){
            this->m_action = Standing;

            if(!m_path.HasPath()){
                if(m_path.Search(m_vecPosition, Enemy::FindPatrolPoint(m_vecPosition))){
                    m_headingTo = Enemy::CenterPosition( m_path.GetNextPoint(m_vecPosition));
                }
            }
            else{
               // this->m_action = Walking; //this happens automatically 
                
                m_headingTo = Enemy::CenterPosition( m_path.GetNextPoint(m_vecPosition));
                this->WalkTowards(m_headingTo);

                if(m_path.ReachedGoal() || ILevelSystem->GetTileAt(m_headingTo)->Blocking() || Vector2::closeEnough(m_vecPosition, Enemy::CenterPosition(m_path.GetGoal()) , 0.2)){
                    m_path.Reset(); 
                    this->m_action = Standing;
                }
            }
            auto player = IEntitySystem->GetLocalPlayer();
            if(isEntityVisible(player, 55)){
                auto ent_pos = player->GetPosition();
                m_view.lookAt(m_vecPosition, ent_pos);
                m_behave.ChangeBehaviour("reposition");
                m_path.Reset();
            }
        
        }});

    m_behave.AddBehaviour("reposition", {
        [this](looptick_t start, looptick_t current){
           
            auto target = IEntitySystem->GetLocalPlayer();
            auto target_pos = target->GetPosition();

            
            if(!m_path.HasPath()){
                if(m_path.Search(m_vecPosition, target_pos)){
                    m_headingTo = Enemy::CenterPosition( m_path.GetNextPoint(m_vecPosition));
                }
                m_repositionrange = Util::SemiRandRange(4.0, 8.0);
            }
           else {
               // this->m_action = Walking; //this happens automatically 
                
               

                if(m_path.ReachedGoal() || ILevelSystem->GetTileAt(m_headingTo)->Blocking() || Vector2::closeEnough(m_vecPosition, Enemy::CenterPosition(m_path.GetGoal()) , m_repositionrange)){
                    m_behave.ChangeBehaviour("aiming"); return;
                }
                 m_headingTo = Enemy::CenterPosition( m_path.GetNextPoint(m_vecPosition));
                this->RunTowards(m_headingTo);
                if(m_path.HalfwayToGoal()){
                    if(m_repositionrange < 4.0)
                        m_repositionrange *= 2.0;
                }
            }

            if(!isEntityVisible(target, 55)){
                
                m_view.lookAt(m_vecPosition, target_pos);
                if(current > start + TICKS_PER_S)
                    m_behave.ChangeBehaviour("patrol");
            }
        
        }});
        m_behave.AddBehaviour("retreat", {
        [this](looptick_t start, looptick_t current){
           
            auto target = IEntitySystem->GetLocalPlayer();
            auto target_pos = target->GetPosition();
          
            
            if(!m_path.HasPath()){
                if(m_path.Search(m_vecPosition,Enemy::FindRetreatPoint(m_vecPosition, target_pos))){
                    m_headingTo = Enemy::CenterPosition( m_path.GetNextPoint(m_vecPosition));
                }
                m_repositionrange = Util::SemiRandRange(3.0, 5.0);
            }
           if(m_path.HasPath()) {
               // this->m_action = Walking; //this happens automatically 
                if(m_path.ReachedGoal() || ILevelSystem->GetTileAt(m_headingTo)->Blocking() || Vector2::closeEnough(m_vecPosition, Enemy::CenterPosition(m_path.GetGoal()), 0.4)){
                    m_behave.ChangeBehaviour("patrol"); return;
                }
                 m_headingTo = Enemy::CenterPosition( m_path.GetNextPoint(m_vecPosition));

                this->RunTowards(m_headingTo);
            }

            if(isEntityVisible(target, 55)){
                
                m_view.lookAt(m_vecPosition, target_pos);
                if(current > (start + TICKS_PER_S) )
                    m_behave.ChangeBehaviour("aiming");
            }
        
        }});
    m_behave.AddBehaviour("aiming", {
        [this](looptick_t start, looptick_t current){
            this->m_action = Aiming;
            auto target = IEntitySystem->GetLocalPlayer();
            auto target_pos = target->GetPosition();
            if(current < start + 6 ){
                 m_view.lookAt(m_vecPosition, target_pos);
                return;
            }
           
            
            
            m_path.Reset();
            if(!isEntityVisible(target, 45)){
                
               // m_view.lookAt(m_vecPosition, target_pos);
                if(current > start + TICKS_PER_S / 2)
                    m_behave.ChangeBehaviour("reposition");
            }
            else if(current > start + TICKS_PER_S / 3)
                    m_behave.ChangeBehaviour("attacking");
            
        
        }});
        m_behave.AddBehaviour("attacking", {
        [this](looptick_t start, looptick_t current){
            this->m_action = Aiming;
            auto target = IEntitySystem->GetLocalPlayer();
            auto target_pos = target->GetPosition();
            if(current > start + 4 ){
                this->m_action = Attacking;
                if(current == start + 6)
                    Attack(target);
            }
            if(current > start + 23 ){
                this->m_action = Aiming;
            }
            if( current > (start + TICKS_PER_S ) ){
                if(Util::SemiRandRange(0,100) > 70){
                    log("%d lucky", GetID());
                     m_view.lookAt(m_vecPosition, target_pos);
                    m_behave.ChangeBehaviour("attacking"); return;
                }
                else if(GetHealth() <= (GetMaxHealth() / 4)){
                        m_path.Reset();
                        m_behave.ChangeBehaviour("retreat"); return;
                    }
                else{
                   
                    if(m_repositionrange < 10.0)
                        m_repositionrange += 1.0;
                    m_behave.ChangeBehaviour("reposition"); return;
                }
            }
            
            
        
        }});


    m_behave.ChangeBehaviour("patrol");
}

void CSoldier::OnWalkToward(const Vector2 &pos)
{
     m_action = Walking; 

     auto curTick = IEngineTime->GetCurLoopTick();
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
  
    looptick_t stepTime = (m_move.m_flForwardSpeed >= 0.09) ? TICKS_PER_S / 4.4 : TICKS_PER_S / 2.3;
    static constexpr double footstepTravel = 4.5;
    if(m_lastFootstep + stepTime < curTick && (IEntitySystem->GetLocalPlayer()->GetPosition() - GetPosition() ).Length2D() < footstepTravel){
      engine->SoundSystem()->PlayPositional(footsound(step_idx), pos, 0.03, 0.1);
      step_idx++;
      m_lastFootstep = curTick;
    }
  
}

void CSoldier::OnLikelyStuck()
{
    if(m_path.HasPath())
        m_path.Reset();
    
}

void CSoldier::OnDestroy() {
    if(m_Texture) delete m_Texture; // hack for editor 
}


void CSoldier::Render(CRenderer *renderer)
{
    draw.camera = renderer->GetActiveCamera();
    CalculateDrawInfo(draw);
    m_anim.Draw(renderer, draw);
}

void CSoldier::OnHit(int damage, const IVector2 &position)
{
    damage *= GetDamageModForHit(position);

    m_health -= damage;
    if(m_action == Dying || m_action == Dead)
        return;
    if(m_health <= 0){
        m_action = Dying;
        m_behave.SetLockedBehaviour("death");
        OnDeath();
    }
    else{
         m_path.Reset();
        if(m_health < (m_maxhealth / 5))
            m_behave.ChangeBehaviour("retreat");
        else
            m_behave.ChangeBehaviour("aiming");
    }
}

bool CSoldier::HitDetect(CCamera *camera, const IVector2 &point, IVector2 *textpos)
{
    return (uint32_t)GetPixelAtPoint(camera, point, textpos) != 0u;
}

float CSoldier::GetDamageModForHit(const IVector2 &pt)
{
    IVector2 dim = m_anim.GetCurrentTextureDimensions();
    /*
        top 1/4 = headshot
        1/4-1/2 = body
        bottom half = legs

    */
  //0,0 == top left

   // log("dim{%d %d} pt{%d %d} t{%d %d}", dim.x, dim.y, pt.x, pt.y, t.x, t.y);
    if(pt.y < 0.25 * dim.y){
       // log("headshot");
        return 1.45f;
    } 
    else if(pt.y < 0.35 * dim.y){
       // log("upper torso");
        return 1.0f;
    } 
    else if(pt.y < 0.5 * dim.y){
       // log("torso");
        return 0.85f;
    } 
    else{
      //  log("legs");
        return 0.65f;
    } 

}

bool CSoldier::Shoot(CBaseEntity *target)
{
     auto pos = target->GetPosition();
     engine->SoundSystem()->PlayPositional ("mp5", m_vecPosition, 0.1, 0.6);
 
    if( CastRayToPoint(pos, target->GetBounds())){
        target->OnHit(m_combat.GetDamage());
        return true;
    }

    return false;
}

void CSoldier::OnDeath()
{
    //play death sound
}

Color CSoldier::GetPixelAtPoint(CCamera *camera, const IVector2 &point, IVector2 *textpos)
{
    draw.camera = camera;
    CalculateDrawInfo(draw);
    return m_anim.GetPixelAtPoint(point, textpos, draw);
}


int CSoldier::DeduceSequenceForOrientation(int *flip, int *anim_state, int *frame, std::string &seq_name) // returns orientation
{

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
    switch (m_action)
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
  
    return orient;
}