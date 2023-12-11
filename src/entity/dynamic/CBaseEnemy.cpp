#include "CBaseEnemy.hpp"
#include <engine/engine.hpp>
#include <util/misc.hpp>


void CBaseEnemy::OnCreate()
{
    ENT_SETUP();
    CreateRenderable();
    m_vecBounds = { 0.07f, 0.3f}; //0.09 0.3
   
    m_iMaxHealth = m_iHealth = 100;
    m_flMoveSpeed = 0.01;

}
inline void CBaseEnemy::OnSetPosition(const Vector2& old_pos, const Vector2& new_pos)
{
   UpdateBBox();
   auto anim_offset = m_anim->GetOffset();

   auto new_offset = Util::getPixelAbsFromAABBOffset(m_lastAnimOffset, m_lastRenderBounds);

   new_offset = m_anim->ScreenToOffset(new_offset);
   m_anim->SetOffset(new_offset);
}



void CBaseEnemy::UpdateBBox()
{
    double sizeMod = std::max(1.0, m_iLastRenderHeight / (SCREEN_HEIGHT / 3.0));
  //  log("%f ", sizeMod);
    const double w = 0.115 * sizeMod, d = 0.05 ; // w 0.15 0.05
    const double x = m_vecPosition.x, y = m_vecPosition.y;

    m_bbox.min = { x - w /2.0, y - d /2.0};
    m_bbox.max = {x + w /2.0, y + d /2.0};
}

void CBaseEnemy::CreateRenderable()
{
    draw_params = {
        .wScale = 1.3, 
        .vScale = 1.3,
        .vOffset = 95 
    };
    SetupTexture("dylan_devred.png");
    SetUpAnimation();
}


void CBaseEnemy::SetupTexture(const std::string &name)
{
    m_Texture = engine->TextureSystem()->FindOrCreatetexture(name);
    SDL_SetSurfaceColorKey(m_Texture->m_texture, SDL_TRUE, Color::Cyan());
    m_hTexture = m_Texture->m_handle;
    m_vecTextureSize = m_Texture->m_size;
    log("%i %i", m_vecTextureSize.w(), m_vecTextureSize.h());
}
void CBaseEnemy::SetUpAnimation()
{
    m_anim = new CAnimController(this, "blood02.png", {"blood",  {42,42 }, { 4, 142, 176, 255}, {4, 142, 176,255}},
        CAnimSequence("default", 1, std::vector<sequence_frame>{
                                                    sequence_frame({0, 0, 1, 1}, 0),
                                                }) ); 
    m_anim->AddSequence(CAnimSequence("blood0", 3, 
         std::vector<sequence_frame>{ 
                                           sequence_frame({22, 117, 42, 42}, 0),
                                           sequence_frame({75, 117, 42, 42}, 1),
                                           sequence_frame({127, 117, 42, 42}, 2),
                                           sequence_frame({175, 117, 42, 42}, 3),
                                           sequence_frame({227, 117, 42, 42}, 4),
                                           sequence_frame({227, 117, 42, 42}, 5),
                                           sequence_frame({281, 117, 42, 42}, 6),
                                           sequence_frame({343, 117, 42, 42}, 7),
                                           sequence_frame({398, 117, 42, 42}, 9),
                                           sequence_frame({505, 117, 42, 42}, 10),
         }), false);
     m_anim->AddSequence(CAnimSequence("blood1", 3, 
         std::vector<sequence_frame>{ 
                                           sequence_frame({44, 117, -42, 42}, 0),
                                           sequence_frame({75, 117, 42, 42}, 1),
                                           sequence_frame({127, 117, 42, 42}, 2),
                                           sequence_frame({175, 117, 42, 42}, 3),
                                           sequence_frame({227, 117, 42, 42}, 4),
                                           sequence_frame({227, 117, 42, 42}, 5),
                                           sequence_frame({281, 117, 42, 42}, 6),
                                           sequence_frame({343, 117, 42, 42}, 7),
                                           sequence_frame({398, 117, 42, 42}, 9),
                                           sequence_frame({505, 117, 42, 42}, 10),
         }), false);
    
     static constexpr auto hBlood = Anim::GetSequenceHandle("blood0");
   m_anim->PlaySequence(hBlood, { 512, 512}); //idk why but this breaks everything if you dont do it so

}

void CBaseEnemy::OnDestroy() {}

void CBaseEnemy::OnHit(int damage, int position)
{
    m_iHealth -= damage;  

    static constexpr auto hBlood = Anim::GetSequenceHandle("blood0");
    if(m_iHealth <= 0)
        return;
    m_flMoveSpeed *= 1.05;
    path.Reset();
    IVector2 hitpos;
 //   auto& box = m_lastRenderBounds;
   // hitpos.x = hitpos.x - SCREEN_WIDTH / 2;
    log("shot %i / %i h", position , m_iLastRenderHeight);

     const int frame_width = m_anim->Drawable()->m_curRect.w; 
    const int frame_height = m_anim->Drawable()->m_curRect.h;
    if(( (float)m_iLastRenderHeight / (float)SCREEN_HEIGHT)  < 0.1)
        return;
    hitpos.x = SCREEN_WIDTH / 2;
    hitpos.y = (SCREEN_HEIGHT / 2) - (position * (0.2 + m_iLastRenderHeight / SCREEN_HEIGHT)) ;
    IVector2 offset;

    offset.x = hitpos.x - SCREEN_WIDTH / 2 + frame_width / 2;
    offset.y = hitpos.y - SCREEN_HEIGHT + frame_height;
    
    m_lastAnimOffset = Util::getPixelOffsetInsideAABB(hitpos, m_lastRenderBounds);
    m_anim->PlaySequence(hBlood, offset ); //NEEDS TO MOVE WITH THEM >:(
    log("playing blood at %i %i", offset.x, offset.y);
   // m_Texture->m_texture->
   // path.Reset();
}

void CBaseEnemy::OnUpdate()
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");

    m_anim->OnUpdate();
    if(m_iHealth <= 0){
        int shit = 0;
        log("I, %i, am dead.", m_iID); 
        path.Reset();
        if(!m_bFrozen)
        {
            auto random = ILevelSystem->FindEmptySpace();
            SetPosition(random.x, random.y);
        }
            
        int coin_toss = Util::SemiRandRange(0,100);
        if(coin_toss  % 2 == 0 && coin_toss > 51){
             auto dup = IEntitySystem->AddEntity<CBaseEnemy>();
             auto random = ILevelSystem->FindEmptySpace();
             dup->SetPosition(random.x, random.y);
        }   
        m_iHealth = m_iMaxHealth;
    }
    if(m_bFrozen)
        return;

    auto localplayer = IEntitySystem->GetLocalPlayer();

    auto local_pos = localplayer->GetPosition();
     double speedMod = (m_iHealth <= 20) ? 1.5 : 1.0;
   // auto delta = local_pos - m_vecPosition;

    IVector2 mapPos = {m_vecPosition.x, m_vecPosition.y};
    bool wander = true;
    Debug(false);
    IVector2 goalPos = (wander) ? ILevelSystem->FindEmptySpace() :  IVector2(local_pos.x, local_pos.y);
    if(!path.HasPath()){
       
        path.Search(mapPos, goalPos );
         dbg("finding a path %i",path.HasPath());
    }
        

    if(path.HasPath()){ //need to make it wait a bit longer because it cuts corners
        m_vecNextPoint = path.GetNextPoint(mapPos);
        if(ILevelSystem->GetMapAt(m_vecNextPoint) != Level::Tile_Empty){
            path.Reset(); return;
        }
         //messy but crucial
        if(mapPos == m_vecNextPoint){
            mapPos = IVector2::Rounded({m_vecPosition.x, m_vecPosition.y});
            if(path.ReachedGoal()){
                auto delta = local_pos - m_vecPosition;
                if(delta.LengthSqr() > 4)
                    path.Reset(); 
                return;
            }
            m_vecNextPoint = path.GetNextPoint(mapPos);
            //dbg("getting new pt {%i %i} {%i %i}", mapPos.x, mapPos.y, next_pt.x, next_pt.y);
             if(mapPos == m_vecNextPoint){
                return;
             }
        }
            
        Vector move = {m_vecNextPoint.x + 0.2, m_vecNextPoint.y + 0.2, 0};
        auto delta = move - m_vecPosition;

        delta =  delta * m_flMoveSpeed * speedMod;

         auto new_pos = m_vecPosition + delta;
        // log("moving to point %f %f", new_pos.x, new_pos.y);
        SetPosition(new_pos.x, new_pos.y, 0);
    }

    

}

void CBaseEnemy::CreateMove(IVector2 dir)
{

}



void CBaseEnemy::Render(CRenderer *renderer)
{
    
    DrawEnemy(renderer,draw_params.wScale, draw_params.vScale, draw_params.vOffset );
    m_anim->DrawFrame(renderer, { 0, 0}, 230);
}
void CBaseEnemy::OnRenderStart(){}
void CBaseEnemy::OnRenderEnd(){}

void CBaseEnemy::CalculateDrawInfo(IVector2 *drawStart,IVector2* drawEnd, IVector2* renderSize, IVector2* screen, Vector2* tform,
                                        CCamera *camera, double wScale, double vScale, int vOffset)
{
     Vector2 relPos = {
         m_vecPosition.x - camera->m_vecPosition.x, 
         m_vecPosition.y - camera->m_vecPosition.y, 
     };
    

    const auto camPlane = camera->m_vecPlane;
    const auto camDir = camera->m_vecDir;
    double invDet = 1.0 / ((camPlane.x * camDir.y) - (camDir.x * camPlane.y));
 
    Vector2 transform;
    // transform sprite with the inverse camera matrix
    //  [ planeX   dirX ] -1                                       [ dirY      -dirX ]
    //  [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
    //  [ planeY   dirY ]                                          [ -planeY  planeX ]
    transform.x = invDet * (camDir.y * relPos.x - camDir.x * relPos.y);
    transform.y = invDet * (-1.0 * (camPlane.y) * relPos.x + camPlane.x * relPos.y);

    int screen_x;
    screen_x = int((SCREEN_WIDTH / 2) * (1 + (transform.x / transform.y) ));
    // parameters for scaling and moving the sprites -> maybe use for custom texture spriteinfo class thing
  

    int vMoveScreen = int(vOffset / transform.y) + camera->m_flPitch + (camera->m_vecPosition.z / transform.y);

    int renderHeight = std::abs(int(SCREEN_HEIGHT / (transform.y))) / vScale; // using transform.y vs real distance prevents fisheye
    m_iLastRenderHeight = renderHeight;
     // calculate width of the sprite
    int renderWidth = abs(int(SCREEN_HEIGHT / (transform.y))) / wScale;

    // calculate lowest and highest pixel to fill in current stripe
    int drawStartY = (-renderHeight / 2) + (SCREEN_HEIGHT / 2) + vMoveScreen;
    if (drawStartY < 0){
        drawStartY = 0;
    }
       
    int drawEndY = (renderHeight / 2) + (SCREEN_HEIGHT / 2) + vMoveScreen;
    if (drawEndY >= SCREEN_HEIGHT)
        drawEndY = SCREEN_HEIGHT - 1;

    
    int drawStartX = (-renderWidth / 2) + screen_x;
    if (drawStartX < 0)
        drawStartX = 0;
    int drawEndX = (renderWidth / 2) + screen_x;
    if (drawEndX >= SCREEN_WIDTH)
        drawEndX = SCREEN_WIDTH - 1;

    m_lastRenderBounds = { { drawStartX, drawStartY}, {drawEndX, drawEndY}};
    m_lastRenderPos = {drawStartX, drawStartY};

    *drawStart = { drawStartX, drawStartY};
    *drawEnd = {drawEndX, drawEndY};
    *renderSize = { renderWidth, renderHeight};
    *screen = { screen_x, vMoveScreen};
    *tform = { transform.x, transform.y};
}

void CBaseEnemy::DrawEnemy(CRenderer *renderer, double wScale, double vScale, int vOffset)
{
     auto camera = renderer->GetActiveCamera();

    IVector2 drawStart,drawEnd, renderSize, screen;
    Vector2 transform;
    CalculateDrawInfo(&drawStart,&drawEnd, &renderSize, &screen, &transform, camera, wScale, vScale, vOffset);
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
     static auto ILightingSystem = engine->CreateInterface<CLightingSystem>("ILightingSystem");
    auto texture = m_Texture->m_texture;
    uint32_t *pixelsT = (uint32_t *)texture->pixels;
    for (int stripe = drawStart.x; stripe < drawEnd.x; stripe++)
    {
        IVector2 tex;
        tex.x = int(256 * (stripe - ( (-renderSize.w() / 2) + screen.x)) * m_vecTextureSize.x / renderSize.w() ) / 256;
        // the conditions in the if are:
        // 1) it's in front of camera plane so you don't see things behind you
        // 2) it's on the screen (left)
        // 3) it's on the screen (right)
        // 4) ZBuffer, with perpendicular distance
        if (transform.y > 0 && stripe > 0 && stripe < SCREEN_WIDTH && transform.y < (renderer->ZBufferAt(stripe)))
        {
            for (int y = drawStart.y; y < drawEnd.y; y++) // for every pixel of the current stripe
            {
                int d = (y - screen.y) * 256 - SCREEN_HEIGHT * 128 + renderSize.h() * 128; // 256 and 128 factors to avoid floats
                tex.y = ((d * m_vecTextureSize.y) / renderSize.h()) / 256;

                Color color = pixelsT[(texture->pitch / 4 * tex.y) + tex.x]; // get current color from the texture

                
                // log("%s", color.s().c_str());
                if(color == Color::None()) continue;

                if(color == Color::Cyan()) continue;
           
                if(color.r() < 45 && color.g() > 244 && color.b() > 244) continue;
                if(m_iHealth < 80) color /= 1.2f;
                else if(m_iHealth < 50) color /= 2.f;
                else if(m_iHealth < 20) color /= 3.5f;
               
                renderer->SetPixel(stripe, y, color);
                 
                ILightingSystem->ApplyLightForTile(ILevelSystem->GetTileAtFast(m_vecPosition.x, m_vecPosition.y), true, true, m_vecPosition, stripe, y);
            }
        }
    }
}

uint32_t CBaseEnemy::GetPixelAtPoint( CCamera* camera, const IVector2 &point, IVector2* textpos)
{
    IVector2 drawStart,drawEnd, renderSize, screen;
    Vector2 transform;
    CalculateDrawInfo(&drawStart,&drawEnd, &renderSize, &screen, &transform, 
                        camera, draw_params.wScale, draw_params.vScale, draw_params.vOffset);

    if (!(drawStart.x <= point.x && point.x <= drawEnd.x ) )
        return 0u;
    if (!(drawStart.y <= point.y && point.y <= drawEnd.y ) )
        return 0u;
    auto texture = m_Texture->m_texture;
    uint32_t *pixelsT = (uint32_t *)texture->pixels;
    int stripe = point.x;
  
    IVector2 tex;
    tex.x = int(256 * (stripe - ( (-renderSize.w() / 2) + screen.x)) * m_vecTextureSize.x / renderSize.w() ) / 256; //256 to avoid floats

    //***likely dont need any of the checks below***
    // conditions in the if are:
    // 1) it's in front of camera plane  2) it's on the screen (left) 3) it's on the screen (right) //REMOVED 4) ZBuffer, with perpendicular distance
    if (transform.y > 0 && stripe > 0 && stripe < SCREEN_WIDTH ) // REMOVED: && transform.y < (renderer->ZBufferAt(stripe)) we shouldnt need bc we will just hit a wall first
    {
        int y = point.y;
        int d = (y - screen.y) * 256 - SCREEN_HEIGHT * 128 + renderSize.h() * 128; // 256 and 128 factors to avoid floats
        tex.y = ((d * m_vecTextureSize.y) / renderSize.h()) / 256;

        uint32_t uColor = pixelsT[(texture->pitch / 4 * tex.y) + tex.x]; // get current color from the texture
        if(!uColor) return 0;
        SDL_Color color = Render::TextureToSDLColor(uColor);
        if(Render::ColorEqualRGB(color, {0, 255, 255, 255})) return 0u;
        if(color.a < 45 && color.g > 244 && color.b > 244) return 0u; //fixes odd mask issues
        
        return Render::SDLColorToWorldColor(color);
    }
    

    return 0;
}
