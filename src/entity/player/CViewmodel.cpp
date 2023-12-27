#include "CViewmodel.hpp"
#include "CPlayer.hpp"

#include <engine/engine.hpp>

void CViewmodel::Render(CRenderer *renderer)
{
    ActiveWeapon()->Render(renderer);
    DrawCrosshair(renderer);
}

CPlayer *CViewmodel::Owner()
{
    return (CPlayer*)m_pParent;
}

const double bobScale = 18.0;
const double bobAmpl = 10.0;

float CViewmodel::GetBobY()
{
    double ampl = (Owner()->m_isMoving) ? bobAmpl : bobAmpl/4;
    double scale = (Owner()->m_isMoving) ? bobScale - 7 : bobScale;
    return ampl * sin( (double)(IEngineTime->GetCurLoopTick() + 0x1337) / scale); //offset from viewbob?
   
}

float CViewmodel::GetBobX()
{
        double ampl = (Owner()->m_isMoving) ? (bobAmpl/2) : (bobAmpl/6);
         double scale = (Owner()->m_isMoving) ? bobScale - 5 : bobScale;
     return ampl * sin( (double)(IEngineTime->GetCurLoopTick() + 80085) / scale); //offset from viewbob?

     //sin(t) - sin(t/2) - sin(t/4) - sin(t/8)
}

void CViewmodel::DrawCrosshair(CRenderer *renderer) //use SDL drawing and do this at full res scale !
{
    // draw crosshair

    static constexpr int crosshair_x = SCREEN_WIDTH / 2;
    static constexpr int crosshair_y = SCREEN_HEIGHT / 2;
    static Color crosshair_color = params.m_crosshairColor;
    
    Color centerpt = (params.m_crosshairGap) ? renderer->GetPixel(crosshair_x, crosshair_y) : (Color)0;

    for (int x = crosshair_x - params.m_crosshairLength; x <= crosshair_x + params.m_crosshairLength; ++x)
        for (int y = crosshair_y - params.m_crosshairWidth; y <= crosshair_y + params.m_crosshairWidth; ++y)
        renderer->SetPixel( x, y,  crosshair_color);
    for (int y = crosshair_y - params.m_crosshairLength; y <= crosshair_y + params.m_crosshairLength; ++y)
        for (int x = crosshair_x - params.m_crosshairWidth; x <= crosshair_x + params.m_crosshairWidth; ++x)
        renderer->SetPixel(x, y,  crosshair_color);
    
    if(params.m_crosshairDot)
        renderer->SetPixel(crosshair_x, crosshair_y,  params.m_crosshairAltColor);
    else if(params.m_crosshairGap)
        renderer->SetPixel(crosshair_x, crosshair_y, centerpt);
}
