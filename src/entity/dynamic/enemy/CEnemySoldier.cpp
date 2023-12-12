#include "CEnemySoldier.hpp"
#include <engine/engine.hpp>

void CEnemySoldier::OnUpdate()
{

    if(m_state == Dying || m_state == Dead)
    {
        static auto IEngineTime = engine->CreateInterface<CEngineTime>("IEngineTime");

        static auto whenToDie = IEngineTime->GetCurLoopTick() + 21;
        if(m_state == Dying && IEngineTime->GetCurLoopTick() > whenToDie ){
             m_state = Dead;
             draw.params.vOffset *= 2.5;
        }
           
        m_anim.OnUpdate();
        return;
    }
    static Vector2 pts[4] = { { 14.5, 21.3}, {14.5, 18.0}, {19.8, 17.7}, {19.4, 21.2} };
    static int goal = 0;
    static bool roundOnce = false;
    if((int)m_vecPosition.x == (int)pts[goal].x && (int)m_vecPosition.y == (int)pts[goal].y){
        if(goal >= 3) {goal = 0; roundOnce = true;}
        else goal++;
    }
     m_state = Default;
    if(!roundOnce){
        
        WalkTowards(pts[goal]);
        m_anim.OnUpdate();
        return;
    }
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    auto player = IEntitySystem->GetLocalPlayer();


    m_state = Attacking;
    m_view.lookAt(m_vecPosition, player->GetPosition());
    Shoot(player);
    m_anim.OnUpdate();

}

void CEnemySoldier::OnCreate()
{
    ENT_SETUP();
    m_move.m_flForwardSpeed = 0.03; //0.130
    m_move.m_flStrafeSpeed = 0.1;
    m_move.m_flSpeedModifier = 1.38;
    m_move.m_flYawSpeed = 0.2;

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
    if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x + m_view.m_dir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false )
        newPos.x += m_view.m_dir.x * moveSpeed;
    if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + m_view.m_dir.y * moveSpeed, m_vecPosition.z}) == false)
        newPos.y += m_view.m_dir.y * moveSpeed;

 
    SetPosition(newPos);
}

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
        seq_name = "death0"; break;
        
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
        engine->log(" walk hit no cases  <%.2f | x%.2f", angle, cross);
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
         engine->log("facing %s <%.2f | x%.2f", magic_enum::enum_name((CAnimDirectional::AnimOrientations(orient))).data(), angle, cross);

    }
    lastOrient = orient;
    return orient;
}


void CEnemySoldier::Shoot(CPlayer *player)
{
    //ray cast??
    //how do we make it "fair"
    auto player_pos = player->GetPosition();
    float player_bounds = player->GetBounds();
    Ray_t ray = {
        .origin = Vector2(GetPosition()),
        .direction = m_view.m_dir.Normalize() //should already be
    };
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    if(Util::RayIntersectsCircle(ray, player_pos, player_bounds))
    {
        //engine->log("in circle");
        int hit = 0;
        Vector2 ray_pos = ray.origin;
        double step = 0.2;
        double dist_to_player = (Vector2(player_pos) - ray.origin).Length();
        IVector2 lastPos = {-1,-1};
        while((Vector2(player_pos) - ray_pos).Length() >= player_bounds)
        {
            engine->log("%.3f %.3f", ray_pos.x, ray_pos.y);
            ray_pos = ray_pos + (ray.direction * step);
          
            if(ray_pos.x < 0 || ray_pos.y < 0) return;
            if((int)floor(ray_pos.x) == lastPos.x && (int)floor(ray_pos.y) == lastPos.y)
                continue;
            lastPos = ray_pos;
            auto tile = ILevelSystem->GetTileAt(lastPos);
            if(tile->m_nType == Level::Tile_Wall)
                return;
            
        }
        player->OnHit(m_stats.m_main_damage);
        engine->log("hit player");
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
