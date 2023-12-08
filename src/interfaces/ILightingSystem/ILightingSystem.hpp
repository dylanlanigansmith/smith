#pragma once
//#pragma GCC optimize("O2")
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>
#include <light/CLight.hpp>
#include <data/level.hpp>
#include <util/misc.hpp>
#include <nlohmann/json.hpp>


#define STEPMAGICNUM 8.f

typedef std::map<std::string, CLight*(*)()> light_reg_t;



class CLightingSystem : public CBaseInterface
{
    friend class CRenderer;
    friend class CEditor;
    friend class LightData;
public:
    CLightingSystem() : CBaseInterface("ILightingSystem") {}
    ~CLightingSystem() override {}
    virtual void OnCreate() override {
        if(GetRegister()){
            status("light classes");
            for(auto it = GetRegister()->begin(); it != GetRegister()->end(); ++it){
                log(it->first);
            }
        }
    }
    virtual void OnShutdown() override { EndLogFileForInstance(); }
    virtual void OnLoopStart() override {}
    virtual void OnLoopEnd() override {}
    virtual void OnRenderStart() override {}
    virtual void OnRenderEnd() override {}
    virtual void OnEngineInitFinish() override;

    virtual void SetupBlending();
    virtual void RegenerateLighting();
    virtual void CalculateLighting();
    virtual nlohmann::json ToJSON();
    virtual void FromJSON(const nlohmann::json& j);
   
    inline Color Blend(Color color1, Color color2, uint8_t weight)
    {

        if (color2 == 0)
            return color1;
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
   
   float inline posToWeight(float pos){
    constexpr double minWeight = 0.1;
        return std::clamp(pow(pos, 4), 1.0, minWeight);
       // return std::max(  pow( std::fabs(pos ),3 ),  minWeight );
   }
    void inline ApplyLightForTile(tile_t *tile, bool posRayX, bool posRayY, const Vector &worldpos, int x, int y)
    {
        const ivec3 vox_pos = tile->worldToSector(worldpos);
          voxel_t *vox = tile->getVoxelAt(vox_pos.x, vox_pos.y, vox_pos.z);
         SetPixel(x, y, vox->m_light); return; /*
        Vector rel_pos = tile->worldToRelative(worldpos);
       // if(x > 630)
          //  log("rel{%.4f %.4f %.4f} world{%.4f %.4f %.4f}", rel_pos.x, rel_pos.y, rel_pos.z, worldpos.x, worldpos.y, worldpos.z);

        const ivec3 vox_pos = tile->relToSector(rel_pos);

        voxel_t *vox = tile->getVoxelAt(vox_pos.x, vox_pos.y, vox_pos.z);
        auto vcenter = tile->getSectorCenterRelativeCoords(vox_pos.x, vox_pos.y, vox_pos.z);
        rel_pos = rel_pos - vcenter;
        Color &vc = vox->m_light;
        if(!params.dynamic) {
             SetPixel(x, y, vc); return;
        }
        // Calculate weights for each direction
        
        float weight_x = posToWeight(rel_pos.x);
        float weight_y = posToWeight(rel_pos.y);
        float weight_z = posToWeight(rel_pos.z);

        // Blend colors from each direction
        Color clr_x = LinearInterpolate(vc, vox->m_neighbors[posRayX ? WEST : EAST], weight_x);
        Color clr_y = LinearInterpolate(vc, vox->m_neighbors[posRayY ? NORTH : SOUTH], weight_y);
        Color clr_z = LinearInterpolate(vc, vox->m_neighbors[rel_pos.z > 0.64f ? 4 : 5], weight_z);

        // Combine the blended colors
        Color blendedColor = CombineBlendedColors(clr_x, clr_y, clr_z); 
        SetPixel(x, y, LinearInterpolate(vc, blendedColor, params.interpFraction));*/
    }

    inline Color LinearInterpolate(const Color &start, const Color &end, float fraction)
    {

        uint8_t interpR = static_cast<uint8_t>(start.r() + fraction * (end.r() - start.r()));
        uint8_t interpG = static_cast<uint8_t>(start.g() + fraction * (end.g() - start.g()));
        uint8_t interpB = static_cast<uint8_t>(start.b() + fraction * (end.b() - start.b()));

        uint8_t interpA = static_cast<uint8_t>(start.a() + fraction * (end.a() - start.a()));
        return Color(interpR, interpG, interpB, interpA);
    }
    inline Color CombineBlendedColors(const Color &clr_x, const Color &clr_y, const Color &clr_z)
    {
        // Combine the colors from each axis
        uint8_t r = (clr_x.r() + clr_y.r() + clr_z.r()) / 3;
        uint8_t g = (clr_x.g() + clr_y.g() + clr_z.g()) / 3;
        uint8_t b = (clr_x.b() + clr_y.b() + clr_z.b()) / 3;
        uint8_t a = (clr_x.a() + clr_y.a() + clr_z.a()) / 3;
        return Color(r, g, b, a);
    }

    template <typename T>
    static CLight* AddLightRegisteredType()
    {
        return _interface()->AddLight<T>();
    }
    
    CLight* AddLightByClassname(const std::string& name){
        auto search = light_class->find(name);
        if(search == light_class->end()){
            Error("could not add light by classname %s", name.c_str()); return nullptr;
        }
        auto light = search->second();
        if(!light){
            Error("this error literally cant happen but %s", name.c_str()); return light; 
          }
        
        log("added light via cname %s", light->GetName().c_str());
        return light;
    }

    template <typename T>
    T *AddLight()
    {
        auto ptr = new T();
        auto light = (CLight *)ptr;
        light_list.emplace(light->GetName() + std::to_string(light_list.size()), light);
        return ptr;
    }


    template <typename T>
    T *AddLight(const Vector &m_vecPosition, const Color &m_color = Color::White(), float m_flBrightness = 1.f, float m_flIntensity = 1.f, float m_flRange = 16.f)
    {
        auto ptr = AddLight<T>();
        auto light = (CLight *)(ptr);
        light->SetColor(m_color);
        light->SetPosition(m_vecPosition);
        light->SetBrightness(m_flBrightness);
        light->SetIntensity(m_flIntensity);
        light->SetRange(m_flRange);
        
        
        status("Added light %s, %s, pos{%.1f, %.1f, %.1f}, brt %.3f, int %.3f, rng%.3f",
               light->GetName().c_str(), m_color.s().c_str(), m_vecPosition.x, m_vecPosition.y, m_vecPosition.z, light->GetBrightness(), light->GetIntensity(), light->GetRange());
        return ptr;
    }

    constexpr static Color MaxDark() { return Color(45, 45, 45, 225); }


    static light_reg_t* GetRegister(){
        if(!light_class) { light_class = new light_reg_t(); }
        return light_class;
    }
protected:
    bool CastRayToPoint(CLight *light, const Vector &point, float maxDistance, float step = 0.5f);
    inline void SetPixel(int x, int y, const Color color)
    {
        int index = (y * m_lightsurface->pitch / 4) + x;
        ((uint32_t *)(m_lightsurface->pixels))[index] = color;
    }

    inline Color GetPixel(int x, int y)
    {
        int index = (y * m_lightsurface->pitch / 4) + x;
        return Color(((uint32_t *)(m_lightsurface->pixels))[index]);
    }
    inline Color MergeLightColors(Color src, Color add)
    {
        // it would be nice to customize this but for now it uses the Color class + operator
        return add + src;
    }
    virtual Color GetLightAtPoint(const Vector &point);
    virtual Color CalculateLightInfluence(CLight *light, const Vector &point);
    virtual Color CombineWithNeighbors(tile_t *tile); // set false just refreshes nbr clrs
    virtual Color CombineWithNeighbors(voxel_t *voxel);
    virtual void CalculateWallFaces(Color dark);
    virtual void FixWallFaces(int method = 0);
    virtual void FindNeighborColors(tile_t *tile, voxel_t *voxel, int x, int y, int z);
  
    virtual void CalculateTileLightData(tile_t *tile);
    virtual void CalculateLerpLightData(tile_t *tile, bool set = true);
    Color getNeighborColor(tile_t *tile, const ivec3 &rel, int dir);

private:
    static CLightingSystem* _interface();

    light_params params;
    SDL_Texture *m_lighttexture;
    SDL_Surface *m_lightsurface;
    std::vector<Vector> tested_points;
    std::unordered_map<std::string, CLight *> light_list;


    static light_reg_t* light_class;
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


