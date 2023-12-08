#pragma once
#include <common.hpp>
#include <interfaces/ITextureSystem/ITextureSystem.hpp>
#include <interfaces/CBaseInterface.hpp>
#include <data/level.hpp>
#define MAP_SIZE 24

class CLevelSystem : public CBaseInterface
{
    friend class CResourceSystem;
    friend class CEditor;
    friend class CLightingSystem; friend class LightData;
public:

    CLevelSystem() : CBaseInterface("ILevelSystem") { }
    ~CLevelSystem() override {}
    virtual void OnCreate() override;
    virtual void OnShutdown() override;
    virtual void OnLoopStart() override;
    virtual void OnLoopEnd() override;
    virtual void OnRenderStart() override;
    virtual void OnRenderEnd() override;
    virtual void OnEngineInitFinish() override;

    int GetMapAt(const IVector2& w) const { return GetMapAt(w.x, w.y); }
    inline int GetMapAt(int x, int y) const{ //type
        return m_Level->GetTileAt(x,y)->m_nType;
    }
    tile_t* GetTileAt(const IVector2& w) { return GetTileAt(w.x, w.y); }
    inline tile_t* GetTileAt(int x, int y){
        return m_Level->GetTileAt(x,y);
    }
    inline tile_t* GetTileAtFast(int x, int y){
        return m_Level->GetTileAtFast(x,y);
    }
    inline  tile_t* GetTileSafe(int x, int y){
        x = std::clamp(x, 0, MAP_SIZE - 1); 
        y = std::clamp(y, 0, MAP_SIZE - 1); 
          return m_Level->GetTileAt(x,y);
    }
    inline virtual bool ValidTilePosition(const IVector2& w) { return ValidTilePosition(w.x, w.y); }
    inline  bool ValidTilePosition(int x, int y){
      return (std::clamp(x, 0, MAP_SIZE - 1) == x &&  y == std::clamp(y, 0, MAP_SIZE - 1)); 
    }
    virtual texture_t* GetTextureAt(int x, int y, uint8_t type = 0); //main0, floor1, ceiling2
    virtual texture_t* GetTexturePlane(bool is_floor, int x, int y);
    virtual void AddBulletHole(tile_t* tile, const IVector2 pos, const uint8_t* side, float radius = 10.f);

    virtual tile_t* GetTileNeighbor(tile_t* tile, int dir);
    virtual IVector2 FindEmptySpace();

    bool IsCollision(const Vector& origin, const Vector& goal); //false = no collision

    auto GetPlayerStart() const { return m_Level->m_vecPlayerStart; }
private:
    void LoadAndFindTexturesForMap();
    void AddMapTexture(int id, const std::string& name);
    hTexture FindTextureForMapObject(int obj);
private:
    CLevel* m_Level;
    CTextureSystem* m_TextureSystem;
    std::unordered_map<int, hTexture> level_textures; //for geometry UNUSED

   

};
