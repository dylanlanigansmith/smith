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

    virtual int GetMapAt(const IVector2& w) const { return GetMapAt(w.x, w.y); }
    virtual int GetMapAt(int x, int y) const{ //type
        return m_Level->GetTileAt(x,y)->m_nType;
    }
    virtual tile_t* GetTileAt(const IVector2& w) { return GetTileAt(w.x, w.y); }
    virtual tile_t* GetTileAt(int x, int y){
        return m_Level->GetTileAt(x,y);
    }

    virtual texture_t* GetTextureAt(int x, int y, uint8_t type = 0); //main0, floor1, ceiling2
    virtual texture_t* GetTexturePlane(bool is_floor, int x, int y);
    virtual void AddBulletHole(tile_t* tile, const IVector2 pos, const uint8_t* side, float radius = 10.f);


    virtual IVector2 FindEmptySpace();
private:
    void LoadAndFindTexturesForMap();
    void AddMapTexture(int id, const std::string& name);
    hTexture FindTextureForMapObject(int obj);
private:
    CLevel* m_Level;
    CTextureSystem* m_TextureSystem;
    std::unordered_map<int, hTexture> level_textures; //for geometry

    int mapold[MAP_SIZE][MAP_SIZE]= {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,69,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
};
