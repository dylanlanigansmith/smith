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
#include <types/Color.hpp>
#include <interfaces/ILevelSystem/ILevelSystem.hpp>
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

        if (color2 == 0u)
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
    void inline ApplyLightForTile1(tile_t *tile, bool posRayX, bool posRayY, const Vector &worldpos, int x, int y)
    {
        const ivec3 vox_pos = tile->worldToSector(worldpos);
          voxel_t *vox = tile->getVoxelAt(vox_pos.x, vox_pos.y, vox_pos.z);
         SetPixel(x, y, vox->m_light); return;
    }
    void inline ApplyLightForTile(tile_t *tile, bool posRayX, bool posRayY, const Vector &worldpos, int x, int y)
    {
        #define DIV 10
        int xIndex = std::clamp(static_cast<int>( std::round(worldpos.x * DIV)), 0, MAP_SIZE * DIV);
        int yIndex = std::clamp(static_cast<int>(std::round(worldpos.y * DIV)), 0, MAP_SIZE * DIV);
        int zIndex = std::clamp(static_cast<int>(std::round(worldpos.z * DIV)), 0, 1 * DIV);
         Color direct = lightmap[xIndex][yIndex][zIndex];

           SetPixel(x, y, direct); return;
           /*
        if(xIndex >= MAP_SIZE * DIV || yIndex >= MAP_SIZE * DIV ){
             SetPixel(x, y, direct); return;
        }

        Color cx = lightmap[xIndex + 1][yIndex][zIndex];
        Color cy = lightmap[xIndex][yIndex + 1][zIndex];
         
         Color combo = CombineBlendedColors(cx,cy,direct);
        
       
         SetPixel(x, y, LinearInterpolate(direct, combo, params.intensityModifier)); return;
       // if( !std::isnan(xIndex) && !std::isnan(yIndex) )
      //      log("{%.4f %.4f %.4f}   %d %d %d %s",worldpos.x,worldpos.y,worldpos.z, xIndex, yIndex, zIndex, direct.s().c_str());*/
        
    }

    Color GetLightAt(const Vector &worldpos){
        int xIndex = std::clamp(static_cast<int>( std::round(worldpos.x * 10.f)), 0, MAP_SIZE * 10);
        int yIndex = std::clamp(static_cast<int>(std::round(worldpos.y * 10.f)), 0, MAP_SIZE * 10);
        int zIndex = std::clamp(static_cast<int>(std::round(worldpos.z * 10.f)), 0, 1 * 10);

        return lightmap[xIndex][yIndex][zIndex];
    }

    inline void ApplyLightForTile2(tile_t *tile, bool posRayX, bool posRayY, const Vector &worldpos, int x, int y)
    {
        

        const ivec3 vox_pos = tile->worldToSector(worldpos);
        Color total_color = MaxDark();
        auto tile_type = tile->m_nType;
        Vector2 pos = worldpos;
        for (auto &light : tile->influential_lights)
        {
            Vector2 light_pos = Vector2(light->GetPosition());
            Ray_t ray = {
                .origin = light_pos,
                .direction = Vector2(worldpos - light->GetPosition()).Normalize()};
            Vector2 rayPos = ray.origin;
            const double step = 0.16;

            int hit = 0;

            const double dist_to_point = (Vector2(pos) - light_pos).Length();
            auto light_tile_pos = IVector2(light_pos);
            IVector2 last_tile_pos = light_tile_pos;
            int walls = 0;
            int iterations = 0;
            int expected_iterations = dist_to_point / step + 1;

            while (1)
            {
                iterations++;
                rayPos = rayPos + (ray.direction * step);
                if (floor(rayPos.x) == last_tile_pos.x && floor(rayPos.y) == last_tile_pos.y && last_tile_pos != tile->m_vecPosition)
                    continue;
                auto ray_tile = LevelSystem->GetTileAt(rayPos.x, rayPos.y);

                if (!ray_tile)
                    break;
                last_tile_pos = ray_tile->m_vecPosition;
                double rayDist = (Vector2(pos) - rayPos).Length();

                if (ray_tile->m_nType == Level::Tile_Wall)
                {
                    walls++;
                    if (ray_tile->m_vecPosition != tile->m_vecPosition || walls > 2)
                    {
                        // failure
                        break;
                    }
                }
                if (tile_type == Level::Tile_Empty || (rayDist < (step * 2)))
                {
                    if (last_tile_pos == tile->m_vecPosition || last_tile_pos == light_tile_pos)
                    {
                        total_color = light->CalculateInfluence(pos, total_color, params, MaxDark());
                        break;
                    }
                }
                if ((rayDist < (step * 2)) && (last_tile_pos == tile->m_vecPosition))
                {

                    total_color = light->CalculateInfluence(pos, total_color, params, MaxDark());
                    break;
                }
            }
        }

        // voxel_t *vox = tile->getVoxelAt(vox_pos.x, vox_pos.y, vox_pos.z);
        SetPixel(x, y, total_color);
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
   
    inline void SetPixel(int x, int y, const Color color)
    {
        const static int pitch = m_lightsurface->pitch / 4;
        int index = (y * pitch) + x;
        ((uint32_t *)(m_lightsurface->pixels))[index] = color;
    }

    inline Color GetPixel(int x, int y)
    {
        const static int pitch = m_lightsurface->pitch / 4;
        int index = (y * pitch) + x;
        return Color(((uint32_t *)(m_lightsurface->pixels))[index]);
    }
    inline Color MergeLightColors(Color src, Color add)
    {
        // it would be nice to customize this but for now it uses the Color class + operator
        return add + src;
    }
   
 
  

    Color getNeighborColor(tile_t *tile, const ivec3 &rel, int dir);

private:
    static CLightingSystem* _interface();
    static CLevelSystem* LevelSystem;
    light_params params;
    SDL_Texture *m_lighttexture;
    SDL_Surface *m_lightsurface;
    std::vector<Vector> tested_points;
    std::unordered_map<std::string, CLight *> light_list;

    std::array<std::array<std::array<Color, 1 * 10>, MAP_SIZE * 10>, MAP_SIZE * 10> lightmap;
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


