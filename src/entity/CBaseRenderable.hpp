#pragma once

#include <common.hpp>
#include <renderer/renderer.hpp>
#include <interfaces/ITextureSystem/ITextureSystem.hpp>

#include "CBaseEntity.hpp"

struct sprite_draw_params{
    double wScale;
    double vScale;
    int vOffset;

    sprite_draw_params() : wScale(1.0), vScale(1.0), vOffset(0) {}

};

struct sprite_draw_data
{
    IVector2 drawStart;
    IVector2 drawEnd; 
    IVector2 renderSize;
    IVector2 screen;
    Vector2 transform;
    CCamera *camera;
    sprite_draw_params params;
     sprite_draw_data() : params() {}
    sprite_draw_data(CCamera* camera) : camera(camera), params() {} 
    sprite_draw_data(CCamera* camera, const sprite_draw_params& params) : camera(camera), params(params) {} 

    //todo delete copy constrtr
};

class CBaseRenderable :  public CBaseEntity
{
public:
    CBaseRenderable(int m_iID) : CBaseEntity(m_iID) {}
    virtual ~CBaseRenderable(){}
    virtual inline bool IsRenderable() { return true; }
    virtual void CreateRenderable() = 0;
    virtual void OnRenderStart() = 0;
    virtual void OnRenderEnd() = 0;
    virtual void Render(CRenderer* renderer) = 0;
    virtual hTexture GetTextureHandle() { return m_hTexture; }
    auto GetTexture() { return m_Texture; }
    virtual bool HasTexture() { return true; }
    
    virtual void CalculateDrawInfo(sprite_draw_data& data)
    {
        Vector2 relPos = {
            m_vecPosition.x - data.camera->m_vecPosition.x, 
            m_vecPosition.y - data.camera->m_vecPosition.y, 
        };
        

        const auto camPlane = data.camera->m_vecPlane;
        const auto camDir = data.camera->m_vecDir;
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
    

        int vMoveScreen = int(data.params.vOffset / transform.y) + data.camera->m_flPitch + (data.camera->m_vecPosition.z / transform.y);

        int renderHeight = std::abs(int(SCREEN_HEIGHT / (transform.y))) / data.params.vScale; // using transform.y vs real distance prevents fisheye
      
        // calculate width of the sprite
        int renderWidth = abs(int(SCREEN_HEIGHT / (transform.y))) / data.params.wScale;

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

        data.drawStart = { drawStartX, drawStartY};
        data.drawEnd = {drawEndX, drawEndY};
        data.renderSize = { renderWidth, renderHeight};
        data.screen = { screen_x, vMoveScreen};
        data.transform = { transform.x, transform.y};
    }
protected:
    hTexture m_hTexture;
    texture_t* m_Texture;
    
};
