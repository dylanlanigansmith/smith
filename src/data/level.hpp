#pragma once
#include <common.hpp>
#include <entity/CBaseEntity.hpp>
#include <types/Vector.hpp>
#include "Texture.hpp"
#include <SDL3/SDL.h>
#include "CBaseSerializable.hpp"
#include <util/hash_fnv1a.hpp>
#include <data/level/level_types.hpp>
//https://lodev.org/cgtutor/raycasting4.html

enum Tile_Texture : uint8_t
{
    TileTexture_Primary = 0,
    TileTexture_Ceiling = 1,
    TileTexture_Floor = 2,
    TileTexture_SIZE,
};


struct tile_state //local tile storage for future use
{
    uint8_t m_iState[3];
    float m_flState[2];
    bool m_bState;
};

struct decal_t
{
    IVector2 texturePosition;
    uint8_t dir[2]; //side x y signs > 0
    uint8_t side;
    float radius;
    decal_t* m_pNextDecal = nullptr;
    //texture_t* texture;
};

struct tile_t
{
    uint32_t id{};
   
    IVector2 m_vecPosition;
    hTexture m_hTexture;//READONLY
    hTexture m_hTextureCeiling;//READONLY
    hTexture m_hTextureFloor;//READONLY
    texture_t* m_pTexture = nullptr; //READONLY
    texture_t* m_pTextureCeiling = nullptr;//READONLY
    texture_t* m_pTextureFloor = nullptr;//READONLY
    tile_state* m_pState  = nullptr;
    float m_flLight = 1.f;
    uint8_t m_nDecals{};
    decal_t* m_pDecals = nullptr;
    float m_flCeiling = 0.f;
    float m_flFloor = 0.f;
    uint8_t m_nType{};
    std::vector<hEntity> m_occupants;

    void UpdateTexture(texture_t* newTexture, Tile_Texture which = TileTexture_Primary)
    {
        switch(which){
            case TileTexture_Primary:
                m_pTexture = newTexture;
                m_hTexture = newTexture->m_handle;
                break;
            case TileTexture_Floor:
                m_pTextureFloor = newTexture;
                m_hTextureFloor = newTexture->m_handle;
                break;
            case TileTexture_Ceiling:
                m_pTextureCeiling = newTexture;
                m_hTextureCeiling = newTexture->m_handle;
                break;
            default:
                return;
        }
    }
    ~tile_t(){
        if(m_nDecals != 0){
            decal_t* current = this->m_pDecals;
            while (current != nullptr) {
                decal_t* next = current->m_pNextDecal;
                delete current;
                current = next;
            }
        }
    }
};

/*
bounds [x,y]
ceilingheight
floorheight

world
{
    tile[]
    tile[]
    tile[]
    etc.
}
//then we still need another one for entities
*/

class CLevel : public CBaseSerializable //i sortof hate this whole implementation
{
public:
    friend class CLevelSystem;
    friend class CEditor;
    CLevel(IVector2 bounds = IVector2(0,0)) : CBaseSerializable(Util::getClassName(this)), m_vecBounds(bounds) {
      m_flCeilingHeight = m_flFloorHeight = 0.0;
      m_szLevelName = "default";
    }
    virtual ~CLevel(){
    }

    void MakeEmptyLevel(hTexture def)
    {
          for(int y = 0; y < m_vecBounds.y; ++y){
            std::vector<tile_t> row;
            for(int x = 0; x < m_vecBounds.x; ++x){
                tile_t empty;
                empty.m_nType = 0;
                if( x == 0 || y == 0 || x == m_vecBounds.x - 1 || y == m_vecBounds.y - 1) 
                    empty.m_nType = 1;
                empty.m_hTexture = empty.m_hTextureCeiling = empty.m_hTextureFloor = def;
                empty.m_vecPosition = IVector2(x,y);
                empty.id = MakeTileID(empty);
                row.push_back(empty);
            }
            world.push_back(row);
        }
    }
    uint32_t MakeTileID(const tile_t& empty){
        return Util::fnv1a::Hash(std::string( 
                    std::to_string(empty.m_vecPosition.x) + std::to_string(empty.m_vecPosition.y)
                    + std::to_string(rand())
                ).c_str()); 
    }

    virtual tile_t* GetTileAt(int x, int y){
     //   log("%i %i", x, y);
        if(x >= 0  && y >= 0 && (x < m_vecBounds.x) && (y < m_vecBounds.y))
            return &(world.at(y).at(x));
        else
            return nullptr;
    }
    virtual void AddTile(const tile_t& tile){
        world.at(tile.m_vecPosition.y).at(tile.m_vecPosition.x)
        = tile;

    }

    virtual json ToJSON()
    {
        auto meta = json::array();
        meta = {m_szLevelName, m_vecBounds.x, m_vecBounds.y, m_flCeilingHeight, m_flFloorHeight};

        auto j = json::object();
        j.emplace("metadata", meta);

        auto tile_data = json::object();
        for(const auto& row : world)
        {
            for(const auto& tile : row)
            {
                auto t = TileToJson(tile);
                tile_data.emplace(std::to_string(tile.id), t);
            }
        }
        j.emplace("tile", tile_data);
        return j;
    }
    virtual bool FromJSON(const json& j ) //entire file
    {
       auto meta = j.at("metadata");
       m_szLevelName = meta.at(0);
       m_vecBounds.x = meta.at(1); m_vecBounds.y = meta.at(2);
       m_flCeilingHeight = meta.at(3); m_flFloorHeight = meta.at(4);

        MakeEmptyLevel(HTEXTURE_INVALID); //now that we have bounds
       auto tile_data = j.at("tile");
        int amt = 0;
       for(const auto& key : tile_data)
       {
            auto tile = JsonToTile(key);
            AddTile(tile);
            amt ++;
       }

       return (amt > 0);
    }
    virtual bool Validate()
    {
        return false;
    }

    virtual void SetMetadata(const std::string& name, double ceil = 0.0, double floor = 0.0){
        m_szLevelName = name;
        m_flCeilingHeight = ceil;
        m_flFloorHeight = floor;
    }
    const auto getName() { return m_szLevelName; }
protected:
    virtual json TileToJson(const tile_t& tile)
    {
         auto t = json::array();
         t = { tile.id, tile.m_vecPosition.x, tile.m_vecPosition.y, tile.m_hTexture, tile.m_hTextureCeiling, tile.m_hTextureFloor,
          tile.m_flLight, tile.m_nDecals, tile.m_flCeiling, tile.m_flFloor, tile.m_nType};
                
        return t;
    }
    virtual tile_t JsonToTile(const json& j){

        return tile_t{
            .id = j.at(0),
            .m_vecPosition = {(int)j.at(1), (int)j.at(2)},
            .m_hTexture = j.at(3),
            .m_hTextureCeiling = j.at(4),
            .m_hTextureFloor = j.at(5),
            .m_pTexture = nullptr,
            .m_pTextureCeiling = nullptr,
            .m_pTextureFloor = nullptr,
            .m_pState = nullptr,
            .m_flLight =  j.at(6),
            .m_nDecals =  j.at(7),
            .m_pDecals = nullptr,
            .m_flCeiling = j.at(8),
            .m_flFloor = j.at(9),
            .m_nType = j.at(10)
        };
    }
    IVector2 m_vecBounds;
    std::vector<std::vector<tile_t>> world;
    std::string m_szLevelName;
    
    double m_flCeilingHeight;
    double m_flFloorHeight;
};

