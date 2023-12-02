#include "CBaseEnemy.hpp"
#include <engine/engine.hpp>
#include <util/misc.hpp>
void CBaseEnemy::OnCreate()
{
    ENT_SETUP();
    CreateRenderable();
    m_vecBounds = { 73.f, 240.f};
    m_iMaxHealth = m_iHealth = 100;
    m_flMoveSpeed = 0.006;
}


void CBaseEnemy::CreateRenderable()
{
    SetupTexture("dylan_devred.png");
    SetUpAnimation();
}

void CBaseEnemy::SetupTexture(const std::string& name)
{
    m_Texture = engine->TextureSystem()->FindOrCreatetexture(name);
    m_hTexture = m_Texture->m_handle;
    m_vecTextureSize = m_Texture->m_size;
}
void CBaseEnemy::SetUpAnimation(){}

void CBaseEnemy::OnDestroy() {}

void CBaseEnemy::OnHit(int damage, int position)
{
    m_iHealth -= damage;
    
}

void CBaseEnemy::OnUpdate()
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    if(m_iHealth <= 0){
        int x = Util::SemiRandRange(1, 23);
        int y = Util::SemiRandRange(1, 23);
        SetPosition(x,y);
        log("I am dead.");
        if(x % 2 == 0 && y % 2 == 0){
             auto dup = IEntitySystem->AddEntity<CBaseEnemy>();
            x = Util::SemiRandRange(1, 23);
            y = Util::SemiRandRange(1, 23);
            dup->SetPosition(x,y);
        }
       
        m_iHealth = m_iMaxHealth;
    }
    

    auto localplayer = IEntitySystem->GetLocalPlayer();

    auto local_pos = localplayer->GetPosition();

    auto delta = local_pos - m_vecPosition;

    double speedMod = (m_iHealth <= 20) ? 1.5 : 1.0;
    delta =  delta * m_flMoveSpeed * speedMod;

    auto new_pos = m_vecPosition + delta;
    SetPosition(new_pos.x, new_pos.y, 0);

}

void CBaseEnemy::CreateMove(IVector2 dir)
{

}



void CBaseEnemy::Render(CRenderer *renderer)
{
    DrawEnemy(renderer);
}
void CBaseEnemy::OnRenderStart(){}
void CBaseEnemy::OnRenderEnd(){}

void CBaseEnemy::DrawEnemy(CRenderer *renderer, double wScale, double vScale, int vOffset)
{
     auto camera = renderer->GetActiveCamera();
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

    IVector2 screen;
    screen.x = int((SCREEN_WIDTH / 2) * (1 + (transform.x / transform.y) ));
    // parameters for scaling and moving the sprites -> maybe use for custom texture spriteinfo class thing
  

    int vMoveScreen = int(vOffset / transform.y) + camera->m_flPitch + (camera->m_vecPosition.z / transform.y);

    int renderHeight = std::abs(int(SCREEN_HEIGHT / (transform.y))) / vScale; // using transform.y vs real distance prevents fisheye
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

   
    int drawStartX = (-renderWidth / 2) + screen.x;
    if (drawStartX < 0)
        drawStartX = 0;
    int drawEndX = (renderWidth / 2) + screen.x;
    if (drawEndX >= SCREEN_WIDTH)
        drawEndX = SCREEN_WIDTH - 1;

    auto texture = m_Texture->m_texture;
    uint32_t *pixelsT = (uint32_t *)texture->pixels;
    for (int stripe = drawStartX; stripe < drawEndX; stripe++)
    {
        IVector2 tex;
        tex.x = int(256 * (stripe - ( (-renderWidth / 2) + screen.x)) * m_vecTextureSize.x / renderWidth) / 256;
        // the conditions in the if are:
        // 1) it's in front of camera plane so you don't see things behind you
        // 2) it's on the screen (left)
        // 3) it's on the screen (right)
        // 4) ZBuffer, with perpendicular distance
        if (transform.y > 0 && stripe > 0 && stripe < SCREEN_WIDTH && transform.y < (renderer->ZBufferAt(stripe)))
        {
            for (int y = drawStartY; y < drawEndY; y++) // for every pixel of the current stripe
            {
                int d = (y - vMoveScreen) * 256 - SCREEN_HEIGHT * 128 + renderHeight * 128; // 256 and 128 factors to avoid floats
                tex.y = ((d * m_vecTextureSize.y) / renderHeight) / 256;

                uint32_t uColor = pixelsT[(texture->pitch / 4 * tex.y) + tex.x]; // get current color from the texture
                if(!uColor) continue;
                SDL_Color color = Render::TextureToSDLColor(uColor);
                if(Render::ColorEqualRGB(color, {0, 255, 255, 255})) continue;
                if(color.a < 45 && color.g > 244 && color.b > 244) continue;
                if(m_iHealth < 80) Render::DarkenSDLColor(color, 1.2f);
                else if(m_iHealth < 50) Render::DarkenSDLColor(color, 2.f);
                else if(m_iHealth < 20) Render::DarkenSDLColor(color, 3.f);
                renderer->SetPixel(stripe, y, color);

            }
        }
    }
}

