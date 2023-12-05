#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>
#include <light/lights.hpp>
#include <data/level.hpp>


struct light_params
{
    float a = 0.17f;
    float b = 0.01f;
    float minIntensity = 0.008;
    float alphaFactorMod = 1.37f;
    float brightFactorMod = 1.9f;
    float finalAlphaMod = 1.1f;
    float interpFraction = 0.77f;
    int mergeMethod = 0; //to be added later
    
};

class CLightingSystem : public CBaseInterface
{
    friend class CRenderer;
    friend class CEditor;
public:
    CLightingSystem() : CBaseInterface("ILightingSystem") { }
    ~CLightingSystem() override {}
    virtual void OnCreate() override {}
    virtual void OnShutdown() override { EndLogFileForInstance(); }
    virtual void OnLoopStart() override {}
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override{}
    virtual void OnRenderEnd() override {}
    virtual void OnEngineInitFinish() override;

    virtual void SetupBlending();
    virtual void RegenerateLighting();
    virtual void CalculateLighting();

    virtual Color GetLightForTile(tile_t* tile);
    virtual void ApplyLightForTile(tile_t* tile, Color src, const Vector& worldpos, int x, int y); //Z(height) scale = 0-3
    
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
        status("Added light %s, %s, pos{%.1f, %.1f, %.1f}, brt %.3f, int %.3f, rng%.3f", 
                light->GetName().c_str() ,m_color.s().c_str(), m_vecPosition.x, m_vecPosition.y, m_vecPosition.z, light->GetBrightness(), light->GetIntensity(), light->GetRange());
        return ptr;
    }

    constexpr static Color MaxDark()  { return Color(45,45,45,225); }
protected:
    bool CastRayToPoint(CLight* light, const Vector& point, float maxDistance, float step = 0.5f);
    inline void SetPixel(int x, int y, const Color color)
    {
        int index = (y * m_lightsurface->pitch / 4) + x;
        ((uint32_t *)(m_lightsurface->pixels))[index] = color;
    }

    inline Color GetPixel(int x, int y)
    {
        int index = (y * m_lightsurface->pitch / 4) + x;
        return Color( ((uint32_t *)(m_lightsurface->pixels))[index]);
    }
    inline Color MergeLightColors(Color src, Color add)
    {
        //it would be nice to customize this but for now it uses the Color class + operator
        return   add + src;
    }
    virtual Color GetLightAtPoint(const Vector& point);
    virtual Color CalculateLightInfluence(CLight* light, const Vector& point);
    virtual Color CombineWithNeighbors(voxel_t* voxel);

    virtual void FindNeighborColors(tile_t* tile, voxel_t* voxel, int x, int y, int z);
    virtual Color LinearInterpolate(Color start, Color end, float fraction);
    virtual void CalculateTileLightData(tile_t* tile);
    virtual void CalculateLerpLightData(tile_t* tile);
    Color getNeighborColor(tile_t* tile, const ivec3& rel, int dir);
private:

    light_params params;
    SDL_Texture* m_lighttexture;
    SDL_Surface* m_lightsurface;
    std::vector<Vector> tested_points;
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