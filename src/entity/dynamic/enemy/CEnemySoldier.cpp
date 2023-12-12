#include "CEnemySoldier.hpp"
#include <engine/engine.hpp>

void CEnemySoldier::OnUpdate()
{
    static Vector2 pts[4] = { { 14.5, 21.3}, {14.5, 18.0}, {19.8, 17.7}, {19.4, 21.2} };
    static int goal = 0;
    if((int)m_vecPosition.x == (int)pts[goal].x && (int)m_vecPosition.y == (int)pts[goal].y){
        if(goal >= 3) goal = 0;
        else goal++;
    }
    m_state = Default;
    WalkTowards(pts[goal]);
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
  
   if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x + m_view.m_dir.x * moveSpeed, m_vecPosition.y, m_vecPosition.z}) == false )
        m_vecPosition.x += m_view.m_dir.x * moveSpeed;
   if(ILevelSystem->IsCollision(m_vecPosition, {m_vecPosition.x,  m_vecPosition.y + m_view.m_dir.y * moveSpeed, m_vecPosition.z}) == false)
        m_vecPosition.y += m_view.m_dir.y * moveSpeed;
  
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
void CEnemySoldier::CreateRenderable()
{
    auto seq_stand = m_anim.AddDefaultSequenceByName("stand0", {64, 64});
    m_anim.AddSequenceByName("walkside0");
    m_anim.AddSequenceByName("walkfwd0");
    m_anim.AddSequenceByName("walkaway0");
    m_anim.AddSequenceByName("walkdiag0");
    m_anim.AddSequenceByName("walkdiag1");
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
