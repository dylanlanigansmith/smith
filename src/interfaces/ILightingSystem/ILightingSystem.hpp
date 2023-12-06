#pragma once
#pragma GCC optimize ("O2")
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>
#include <light/lights.hpp>
#include <data/level.hpp>

#define STEPMAGICNUM 8.f
struct light_params
{
    float a = 0.14f;
    float b = 0.01f;
    float minIntensity = 0.009;
    float alphaFactorMod = 1.67f;
    float brightFactorMod = 0.65f;
    float finalAlphaMod = 0.52f;
    float interpFraction = 0.41f;
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
    inline Color Blend(Color color1, Color color2, uint8_t weight)
    {
         static constexpr Color cnull = Color::None();
         if(color2 == cnull) return color1;
        // Extract RGBA components
        uint8_t r1 = (color1 >> 24) & 0xFF;
        uint8_t g1 = (color1 >> 16) & 0xFF;
        uint8_t b1 = (color1 >> 8) & 0xFF;
        uint8_t a1 = color1 & 0xFF;

        uint8_t r2 = (color2 >> 24) & 0xFF;
        uint8_t g2 = (color2 >> 16) & 0xFF;
        uint8_t b2 = (color2 >> 8) & 0xFF;
        uint8_t a2 = color2 & 0xFF;

        // Blend each component using integer arithmetic
        uint8_t r = (r1 * (255 - weight) + r2 * weight) >> 8;
        uint8_t g = (g1 * (255 - weight) + g2 * weight) >> 8;
        uint8_t b = (b1 * (255 - weight) + b2 * weight) >> 8;
        uint8_t a = (a1 * (255 - weight) + a2 * weight) >> 8;

        // Combine back into uint32_t
        return (r << 24) | (g << 16) | (b << 8) | a;
    }
    void inline  ApplyLightForTile(tile_t* tile, bool posRayX, bool posRayY, const Vector& worldpos, int x, int y) //Z(height) scale = 0-3
    {
        Vector rel_pos = tile->worldToRelative(worldpos);
        const ivec3 vox_pos = tile->relToSector(rel_pos);
       // static const Vector epsilon = Vector(0.01f,0.01f,0.01f);
       // rel_pos = rel_pos + epsilon;
       
        //log("world{%.3f %.3f %.3f}, vox[%i %i %i], screen(%i %i) tile: [%i] @ [%i,%i]",
         //    worldpos.x, worldpos.y, worldpos.z, vox_pos.x, vox_pos.y, vox_pos.z, x, y, tile->m_nType, tile->m_vecPosition.x, tile->m_vecPosition.y);

        voxel_t* vox = tile->getVoxelAt(vox_pos.x, vox_pos.y, vox_pos.z); // we can probably do this unsafe w.out wrapper bc relToSector coords clamps them
        Color& vc = vox->m_light;
        SetPixel(x, y, vc);
        //Color bld; //bc short names are faster right??? lol
  
        /*
            posRayX = west nbr >>>
            posRayY = north nbr >>>
            rel_pos.z > 0.6 down nbr >>>
       
        uint8_t weight_x = ( (uint8_t)(rel_pos.x * 255) );
        uint8_t weight_y = ( (uint8_t)(rel_pos.y * 255) );
        uint8_t weight_z = ( (uint8_t)(rel_pos.z * 255) );
        // north, east, south, west, up = 4, down = 5
        //unrolled
        Color clr_x = (posRayX) ? Blend(vc, vox->m_neighbors[WEST], weight_x) : Blend(vc, vox->m_neighbors[EAST], weight_x);
        Color clr_y = (posRayX) ? Blend(vc, vox->m_neighbors[NORTH], weight_y) : Blend(vc, vox->m_neighbors[SOUTH], weight_y);
        Color clr_z =  clr_x + clr_y;// (rel_pos.z > 0.6f) ? Blend(vc, vox->m_neighbors[5], weight_z) : Blend(vc, vox->m_neighbors[4], weight_z);
        uint8_t r,g,b,a;
        r = (clr_x.r() + clr_y.r() + clr_z.r()) / 3.f;
        g = (clr_x.g() + clr_y.g() + clr_z.g()) / 3.f;
        b = (clr_x.b() + clr_y.b() + clr_z.b()) / 3.f;
        a = (clr_x.a() + clr_y.a() + clr_z.a()) / 3.f;

        bld = Color(r,g,b,a);
        SetPixel(x, y, bld); */
    }
    
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
    virtual Color CombineWithNeighbors(tile_t* tile); //set false just refreshes nbr clrs
    virtual void CalculateWallFaces(Color dark);
    virtual void FixWallFaces(int method = 0);
    virtual void FindNeighborColors(tile_t* tile, voxel_t* voxel, int x, int y, int z);
    virtual Color LinearInterpolate(Color start, Color end, float fraction);
    virtual void CalculateTileLightData(tile_t* tile);
    virtual void CalculateLerpLightData(tile_t* tile, bool set = true);
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