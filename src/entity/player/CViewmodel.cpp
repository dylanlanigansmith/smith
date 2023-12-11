#include "CViewmodel.hpp"
#include "CPlayer.hpp"
void CViewmodel::Render(CRenderer *renderer)
{
    ActiveWeapon()->Render(renderer);
    DrawCrosshair(renderer);
}

CPlayer *CViewmodel::Owner()
{
    return (CPlayer*)m_pParent;
}

void CViewmodel::DrawCrosshair(CRenderer *renderer)
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
