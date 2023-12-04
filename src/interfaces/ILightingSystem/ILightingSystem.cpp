#include "ILightingSystem.hpp"
#include <interfaces/ILevelSystem/ILevelSystem.hpp>
#include <engine/engine.hpp>

constexpr Color MaxDark = Color(45,45,45,245);

constexpr double g_flDistanceMod = 0.2;

Color CLightingSystem::GetLightForTile(tile_t *tile)
{
    static auto ILevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");

    Vector tile_pos = { tile->m_vecPosition.x + 0.5f, tile->m_vecPosition.y + 0.5f, 0.25f};
    Color total_light = MaxDark;
    float total_brightness = 1.f;
    for(auto& entry : light_list){
        Color to_set = Color::None();
        auto light = entry.second;

        auto light_pos = light->GetPosition();
        auto delta = light_pos - tile_pos;
        float range = light->GetRange();
        const float a = 0.2f, b = 0.02, minIntensity = 0.1; //wow this is hard

        float distanceSquared = delta.LengthSqr();
        float attenuation = 1.0f / (1.0f + a * sqrt(distanceSquared) + b * distanceSquared);
        attenuation = std::min(std::max(attenuation, minIntensity), 1.0f); // Clamp attenuation

        Color lightColor = light->GetColor(); 
        lightColor.a( lightColor.a() * attenuation / light->GetIntensity() * light->GetBrightness() * 10);
      
       
        total_light = MergeLightColors(total_light, lightColor);
        /*
        if(delta.LengthSqr() <= g_flDistanceMod * range * range ){
            to_set = light->GetColor();
            to_set.a( to_set.a() / light->GetIntensity() * light->GetBrightness());

        }*/

        if(to_set == Color::None()) continue;
      
        total_light = MergeLightColors(total_light, to_set);
        
    }
   // uint8_t alpha = (uint8_t)std::clamp( int(total_light.a() *   ( 1.f - total_brightness  )) + 120, 0,255 );
    //total_light.a(255 - alpha);
    return total_light;
}
Color CLightingSystem::MergeLightColors(Color src, Color add)
{
    return  src + add;
}
Color CLightingSystem::ApplyLightForTile(tile_t *tile, Color src)
{
    
    auto lit_color = GetLightForTile(tile);
    if(lit_color != Color::None())
        return ApplyLighting(src, lit_color);
    return src;
}

Color CLightingSystem::ApplyLighting(Color src, Color light)
{

    auto c =    light + src;
    // log("src%s light %s ret%s",  src.s().c_str(), light.s().c_str(), c.s().c_str() );
    c.a(255);
    return c;
}



Color CLightingSystem::CalculateLightInfluence(CLight *light, const Vector &point)
{

    return Color::None();
}
