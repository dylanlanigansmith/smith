#pragma once
#include <common.hpp>
#include <entity/CBaseEntity.hpp>
#include <types/Vector.hpp>
#include "Texture.hpp"
#include <SDL3/SDL.h>
#include "CBaseSerializable.hpp"
#include <util/hash_fnv1a.hpp>
#include <data/level/level_types.hpp>

#include <types/Color.hpp>
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


struct voxel_t{
    uint8_t m_nCollision;

    //lighting
    Color m_light = Color::None();
    uint8_t m_neighborsize = 6;
    Color m_neighbors[6] = {
        Color::None(),Color::None(),Color::None(),
        Color::None(),Color::None(),Color::None()
    };

};

//future: a class with operator overloads for tile & vec pair 
struct ivec3{ //dont use
    int x,y,z;

    ivec3 operator+(const ivec3& rhs) const{
        return {x + rhs.x, y + rhs.y, z + rhs.z };
    };
    ivec3 operator%(const int rhs) const{
        return {x % rhs, y % rhs, z % rhs };
    };
};
#define TILE_SECTORS 3

enum Tile_Flags : int
{
    TileFlags_NONE = 0,
    TileFlags_NOCLIP = 1,
};

struct tile_t
{
    //$ = memory size 1/$ $ $$ $! $16 ... | 1 byte 4 byte 8 byte 16 bytes ...
    //# vars also saved

    uint32_t id{};          //$#
   
    IVector2 m_vecPosition;     //$$#
    hTexture m_hTexture;//READONLY      //$#
    hTexture m_hTextureCeiling;//READONLY   //$#
    hTexture m_hTextureFloor;//READONLY     //$#
    texture_t* m_pTexture = nullptr; //READONLY @file //$!
    texture_t* m_pTextureCeiling = nullptr;//READONLY //$!
    texture_t* m_pTextureFloor = nullptr;//READONLY   //$!
    tile_state* m_pState  = nullptr;                  //$!
    float m_flLight = 1.f;      //$# //not usable
    uint8_t m_nDecals{};        //1/$#
    decal_t* m_pDecals = nullptr;  //$8 
    float m_flCeiling = 0.f;        //$#
    float m_flFloor = 0.f;          //$#
    uint8_t m_nType{};              //1/$#
    uint64_t m_nFlags;              //$!
    std::vector<hEntity> m_occupants;
    voxel_t sectors[TILE_SECTORS][TILE_SECTORS][TILE_SECTORS];
    bool IsThinWall(){
        switch(m_nType)
        {
            case Level::Tile_Door:
            case Level::Tile_WallN:
            case Level::Tile_WallE:
            case Level::Tile_WallS:
            case Level::Tile_WallW:
                return true;
            default:
                return false;
        }
    }
    inline bool isEmpty(){
        return (m_nType == Level::Tile_Empty);
    }
    inline int round(float x){
        return f(x + 0.5f);
    }
    inline int f(float x)
    {
        return (int) x - (x < (int) x); // as dgobbi above, needs less than for floor
    }
    inline ivec3 worldToSector(const Vector& worldpos){
        return ivec3{
            std::clamp(static_cast<int>(round((worldpos.x - m_vecPosition.x) * 3.f)), 0, 2), 
            std::clamp(static_cast<int>(round((worldpos.y - m_vecPosition.y) * 3.f)), 0, 2), 
            std::clamp(static_cast<int>(round(worldpos.z * 3.f)), 0, 2)
        };   
    }
    
     inline ivec3 relToSector(const Vector& pos){
        return ivec3{
            std::clamp(static_cast<int>(round(pos.x * 3.f)), 0, 2), 
            std::clamp(static_cast<int>(round(pos.y * 3.f)), 0, 2), 
            std::clamp(static_cast<int>(round(pos.z * 3.f)), 0, 2)
        };    
    }
    inline Vector worldToRelative(const Vector& worldpos){     
        return Vector(
            (worldpos.x - m_vecPosition.x),
            (worldpos.y - m_vecPosition.y),
            worldpos.z
        );
        /*
        return ivec3{
            std::clamp((worldpos.x - tile_pos.x) * 3.f, 0.f, 2.f), 
            std::clamp((worldpos.y - tile_pos.y) * 3.f, 0.f, 2.f), 
            std::clamp(worldpos.z * 3.f, 0.f, 2.f)
        };*/
        
    }

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
    inline Vector getSectorCenterRelativeCoords(const ivec3& v) { return getSectorCenterRelativeCoords(v.x,v.y,v.z); }
    inline Vector getSectorCenterRelativeCoords(int x, int y, int z) {
       
        float voxelSize = _sectoroffset(); //1.f/3.f

        float centerX = (x + 0.5f) * voxelSize;
        float centerY = (y + 0.5f) * voxelSize;
        float centerZ = (z + 0.5f) * voxelSize;

        return Vector(centerX, centerY, centerZ);
    }

    inline voxel_t* getVoxelAt(int x, int y, int z){
      //  assert( 0 <= x && x < 3 && 0 <= y && y < 3 && 0 <= z && z < 3);
      if(! ( 0 <= x && x < 3 && 0 <= y && y < 3 && 0 <= z && z < 3)){
        x = std::clamp(x,0,2); y = std::clamp(y,0,2);z = std::clamp(z,0,2);
      }
        return &(sectors[x][y][z]);
    }
    static constexpr float _sectoroffset() { return 1.f / (float)TILE_SECTORS ; }


    inline void SetNoClip(bool v) { m_nFlags = (v) ? 1 : 0; }
    inline bool NoCollision() const { return (m_nFlags > 0); }

    inline bool Blocking(){
        if(m_nType == Level::Tile_Wall)  return true;
        if(m_nType == Level::Tile_Empty || NoCollision()) return false;

        return true;
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
    friend class CLightingSystem; friend class LightData;
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
                empty.m_vecPosition = IVector2(x,y); empty.m_nFlags = 0;
                empty.m_flFloor = 0.5f; empty.m_flCeiling = 0.5f;
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

    inline tile_t* GetTileAt(int x, int y){
     //   log("%i %i", x, y);
        if(x >= 0  && y >= 0 && (x < m_vecBounds.x) && (y < m_vecBounds.y))
            return &(world[y][x]);
        else
            return nullptr;
    }
    inline tile_t* GetTileAtFast(int x, int y){
     //   log("%i %i", x, y);
        
        return &(world[y][x]);
       
    }
    virtual void AddTile(const tile_t& tile){
        world.at(tile.m_vecPosition.y).at(tile.m_vecPosition.x)
        = tile;

    }

   virtual bool FromJSON(const json& j );
    virtual json ToJSON();
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

    auto& GetWorld() { return world;}
protected:
    virtual json TileToJson(const tile_t& tile)
    {
         auto t = json::array();
         t = { tile.id, tile.m_vecPosition.x, tile.m_vecPosition.y, tile.m_hTexture, tile.m_hTextureCeiling, tile.m_hTextureFloor,
          tile.m_flLight, tile.m_nDecals, tile.m_flCeiling, tile.m_flFloor, tile.m_nType, tile.m_nFlags};
                
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
            .m_nType = j.at(10),
            .m_nFlags = j.at(11)
        };
    }

protected:

    IVector2 m_vecBounds;
    std::vector<std::vector<tile_t>> world;
    std::string m_szLevelName;
    
    Vector2 m_vecPlayerStart;
    uint64_t m_nPlayerInfo;
    double m_flCeilingHeight;
    double m_flFloorHeight;
};

