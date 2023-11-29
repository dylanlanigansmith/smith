#pragma once

#include <common.hpp>
#include <entity/CBaseRenderable.hpp>
#include <entity/CBaseEntity.hpp>
#include <engine/engine.hpp>




class CBaseProp : public CBaseRenderable
{
public:
    CBaseProp(int m_iID);
    virtual ~CBaseProp() {}
    virtual void OnUpdate() {}
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void CreateRenderable() = 0;
    virtual void OnRenderStart() {} 
    virtual void OnRenderEnd() {}
    virtual void Render(CRenderer* renderer) = 0;
protected:
    virtual void SetupTexture(const std::string& name);
    virtual void DrawProp(CRenderer* renderer, double wScale = 1.0, double vScale = 1.0, int vOffset = 0.0);
};

CBaseProp::CBaseProp(int m_iID) : CBaseRenderable(m_iID)
{
    ENT_SETUP();
}

void CBaseProp::DrawProp(CRenderer* renderer, double wScale, double vScale, int vOffset) 
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

    auto ITextureSystem = engine->TextureSystem(); // inconsistent
    auto texture = ITextureSystem->GetTexture(m_hTexture);
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
                SDL_Color color = Render::TextureToSDLColor(uColor);
                if ( (color.r != 0) && (color.g != 0) && (color.b != 0))
                {
                    renderer->SetPixel(stripe, y, color);

                    // paint pixel if it isn't black, black is the invisible color
                }
            }
        }
    }
}

void CBaseProp::SetupTexture(const std::string& name)
{
    auto ITextureSystem = engine->TextureSystem(); // inconsistent
    this->m_hTexture = ITextureSystem->LoadTexture(name);
    auto text = ITextureSystem->GetTexture(m_hTexture);
    this->m_vecTextureSize = {text->w, text->h};
}