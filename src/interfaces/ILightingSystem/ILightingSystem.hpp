#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>
#include <light/lights.hpp>
#include <data/level.hpp>


class CLightingSystem : public CBaseInterface
{
public:
    CLightingSystem() : CBaseInterface("ILightingSystem") { }
    ~CLightingSystem() override {}
    virtual void OnCreate() override {}
    virtual void OnShutdown() override {}
    virtual void OnLoopStart() override {}
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override{}
    virtual void OnRenderEnd() override {}
    
    virtual Color GetLightForTile(tile_t* tile);
    virtual Color ApplyLightForTile(tile_t* tile, Color src);
    virtual Color ApplyLighting(Color src, Color light);
    template <typename T> 
    T* AddLight(const Vector& m_vecPosition, const Color& m_color = Color::White(), float m_flBrightness = 1.f, float m_flIntensity = 1.f, float m_flRange = 16.f){
        auto ptr =  new T();
        auto light = (CLight*) ptr;
        light->SetColor(m_color);
        light->SetPosition(m_vecPosition);
        light->SetBrightness(m_flBrightness);
        light->SetIntensity(m_flIntensity);
        light->SetRange(m_flRange);
        light_list.emplace(light->GetName() + std::to_string(light_list.size()), light);
        log("Added light %s, %s, {%.1f, %.1f, %.1f}", light->GetName().c_str() ,m_color.s().c_str(), m_vecPosition.x, m_vecPosition.y, m_vecPosition.z);
        return ptr;
    }
protected:
    virtual Color MergeLightColors(Color src, Color add);
    virtual Color CalculateLightInfluence(CLight* light, const Vector& point);
private:
   std::unordered_map<std::string, CLight*> light_list;
};

/*
So... do we do real time or baked
- bake for walls

-shadows for entities and props seem easy, walls not so much

Top of render loop calls Light sys -> calculate

then everything asks for its light value

for tiles:
every tile has  a light emission value
-can be from a face if it is a wall or from ceiling 
ray trace 0.5 step to tiles in radius 
difuse effect over 0.5 steps

lighting first then shadows i guess

*/