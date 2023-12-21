#include "ILevelSystem.hpp"
#include <engine/engine.hpp>






#include <data/level.hpp>
#include <util/misc.hpp>
#include <renderer/render_helpers.hpp>

#include <light/lights.hpp>

#include <entity/dynamic/CBaseEnemy.hpp>
#include <entity/dynamic/enemy/CEnemySoldier.hpp>
#include <entity/prop/objects/CBarrel.hpp>
#include <entity/prop/objects/CGreenLight.hpp>
#include <entity/prop/objects/CPillar.hpp>
#include <entity/level/CBaseDoorControl.hpp>
/*
For Editor:

Level Stuff:
We need to serialize the level class
Need to switch functions to new format

**Texture stuff:
**need to create texture database json thing [DONE]
**functions for finding textures [DONE]
final thing is clean up implementation once level stuff done

Tile stuff:
tiles should have decal support -> dont worry about it
lighting
-> make lightmap in editor

ImGui stuff:
cool way to render each tile (imgui selectable?)
ensure we can load textures for imgui as well
new system for it

Engine stuff:
Global states 
ie. in editor, 
add a menu 


Add editor, then add more entities
then add AI
then add a gun 

then network it
then add sound
then it is done


*/


void CLevelSystem::OnCreate()
{
  // m_Level = new CLevel({24,24});
  // m_Level->MakeEmptyLevel(-1);
    m_levelState = LevelSystem_Init;
    m_Level == nullptr;

    static constexpr auto event_change = CEventManager::EventID("change_level");
    IEntitySystem->Events()->AddEvent(event_change);
    eventCallbackFn change_fn = [this](uint32_t caller, event_args args){
        try {
            std::string name = std::any_cast<std::string>(*args);  
            this->log("recieved event 'change_level' %s", name.c_str());
            LoadLevel(name);
            return;
        } catch (const std::bad_any_cast& e) {
            // Handle or log the exception
            this->log("Error in event argument casting: %s expected %s", e.what(), (*args).type().name() );
        }
    };
    IEntitySystem->Events()->AddListener(-1, event_change, change_fn);
}
bool CLevelSystem::LoadLevel(const std::string &map_name)
{

    bool changing = (m_Level != nullptr && m_levelState == LevelSystem_Loaded);
    note("%s level %s!", changing ? "changing to" : "loading", map_name.c_str());
    m_levelState = LevelSystem_Init;

    



    if(changing)
    {
        delete m_Level; m_Level = nullptr;
        IEntitySystem->RemoveAllButPlayer();
         ILightingSystem->OnPreLevelChange(); //clears light list 
    } 
    else
    {

    }
    if(!IResourceSystem->LoadLevel(map_name)) //lvldev_light
        return false;
    log("loaded %s.json", map_name.c_str());
    LoadAndFindTexturesForMap();
    
    
    auto pstart = GetPlayerStart();
    
    
    IEntitySystem->GetLocalPlayer()->SetPosition(pstart.x, pstart.y, IEntitySystem->GetLocalPlayer()->GetPosition().z);

    
    auto barrel = IEntitySystem->AddEntity<CBarrel>();
    barrel->SetPosition(15.7, 9.5);
   

    //19 16
    if(m_Level->getName().compare("lvldeath") != 0)
    {
        // auto light = IEntitySystem->AddEntity<CGreenLight>();
    // light->SetPosition(19.5, 18.5);
        auto pillar = IEntitySystem->AddEntity<CPillar>();
        pillar->SetPosition(2, 12);
        auto doorctl = IEntitySystem->AddEntity<CBaseDoorControl>();
        doorctl->SetTarget({19,16});

        auto doorctl2 = IEntitySystem->AddEntity<CBaseDoorControl>();
        doorctl2->SetTarget({16,3});
        doorctl2->GetDoor().params.m_direction = door_data::DoorDir_RightToLeft;

        for(int i = 0; i < 35; ++i)
        {
            auto sold = IEntitySystem->AddEntity<CEnemySoldier>();
            auto empty = FindEmptySpace();
            while ((pstart - Vector2(empty.x, empty.y)).Length() < 12 ){
                empty = FindEmptySpace();
            }
            sold->SetPosition(empty.x + 0.2, empty.y + 0.3);
            sold->GetPathFinder()->Debug(false);
            if(i % 3 == 0)
                sold->SetType(CEnemySoldier::Soldier_Grunt);
            if(i % 5 == 0)
                sold->SetType(CEnemySoldier::Soldier_Med);
        }
    }   
    
    

    


    if(changing)
    {
       
        //imagine an event system
        //imagine...
         ILightingSystem->RegenerateLighting();
    }
    else
    {
         ILightingSystem->CalculateLighting();
    }
   

   engine->SoundSystem()->PlaySound("Cat", 0.3f);
   m_levelState = LevelSystem_Loaded;

   note("load level %s success", m_Level->getName().c_str());
    return true;
}

void CLevelSystem::OnEngineInitFinish()
{


    LoadLevel("lvldev");
}

tile_t *CLevelSystem::GetTileNeighbor(tile_t *tile, int dir) //nullptr if none
{
    IVector2 coords = tile->m_vecPosition;
    switch(dir)
    {
        case NORTH:
            coords.y -= 1; break;
        case EAST:
            coords.x += 1;break;
        case SOUTH:
            coords.y += 1;break;
        case WEST:
           coords.x -= 1;break;
        default:
            return nullptr;
    }

    auto neighbor = GetTileAt(coords);
    return neighbor;
}

IVector2 CLevelSystem::FindEmptySpace() // Random
{
    int bail = 0;
    while(1)
    {
        if(bail > 100 ){
            Error("findemptyspace bailed after %i tries- returning {1,1}", bail);
            return { 1,1 }; break;
        }

        int x = Util::SemiRandRange(0, MAP_SIZE - 1);
        int y = Util::SemiRandRange(0, MAP_SIZE - 1);
        bail++;
        if(auto t = GetTileAtFast(x,y); t->Blocking() || !t->m_occupants.empty())
            continue;
        return {x,y};
    }
}

bool CLevelSystem::IsCollision(CBaseEntity* caller, const Vector& origin, const Vector& goal)
{
    //to raycast everything we would have to either dda or deduce direction and get face coords, or check all 4 lol

  //  return IsWallCollision(origin, goal);
    if(IsWallCollision(origin, goal)) return true;
    Vector delta = goal - origin;
    auto tile = GetTileAt({goal.x, goal.y});
    auto tile_start = GetTileAt(Vector2(origin));
    if(!caller) return true;
    if(!tile->m_occupants.empty())
    {

        if(tile == tile_start && tile->m_occupants.size() == 1) //its us
            return false;
        
        for(auto& id : tile->m_occupants)
        {
            auto ent = IEntitySystem->GetEntity(id);
            if(!ent) continue; //we got issues 
            if(ent->GetID() == caller->GetID()) continue;
            Vector2 pos = ent->GetPosition();
            auto bounds = ent->GetBounds() + 0.1;
            if( (pos - Vector2(goal)).LengthSqr() <=  bounds*bounds){
            //    warn("too long %f %f", (pos - Vector2(origin)).LengthSqr(),  bounds*bounds);
                ent->WhenCollidedBy(caller);
                caller->OnCollisionWith(ent);
                
                if(ent->IsBlocking())
                    return true;
            }
        }
    }
    return false;


}

bool CLevelSystem::IsWallCollision(const Vector& origin, const Vector& goal)
{
    //to raycast everything we would have to either dda or deduce direction and get face coords, or check all 4 lol
   
    Vector delta = goal - origin;
    auto tile = GetTileAt({goal.x, goal.y});
    auto tile2 = GetTileAt(Vector2(goal - delta * 0.1));
    if(!tile || !tile2) return true;
    auto type = tile->m_nType;
    auto type2 = tile2->m_nType;

    if(!tile->Blocking() ) return false; //idek
    if(tile->Blocking() ) return true;
    
    

    //we have a fancy wall aw shit.. this is likely void w/ flags
    Ray_t ray = {
        .origin = origin,
        .direction = (goal - origin).Normalize()
    };

    auto wall = Render::GetLineForWallType(tile->m_vecPosition, type);
    Vector2 intersect;
    const double wall_thick = 0.1;
                BBoxAABB thickness = {
                .min = wall.p0,
                .max = wall.p1
                };
    if(!Util::RayIntersectsLineSegment(ray, wall, intersect))
        return false;
   // if(!Util::RayIntersectsBox(ray, thickness))
    //    return false;
    if( (Vector2(goal) - intersect).Length() > 0.1)
        return false;
    return true;

}

void CLevelSystem::LoadAndFindTexturesForMap()
{
    auto& world = m_Level->world;
    for(auto& row : world)
    {
        for(auto& tile : row){
           tile.m_pTexture = ITextureSystem->GetTextureData(tile.m_hTexture);
           tile.m_pTextureCeiling = ITextureSystem->GetTextureData(tile.m_hTextureCeiling);
           tile.m_pTextureFloor = ITextureSystem->GetTextureData(tile.m_hTextureFloor);

           if(tile.m_nType == Level::Tile_Wall){
            tile.m_pTextureCeiling = tile.m_pTextureFloor = tile.m_pTexture;
            
           }
        }
    }

//shit i forgot about ceilings
/*
    AddMapTexture(100, "colorstone.png"); //ceiling
    AddMapTexture(101, "bluestone.png"); //check1
    AddMapTexture(102, "purplestone.png"); //check2

    AddMapTexture(1, "wood.png");
    AddMapTexture(2, "redbrick.png");
    AddMapTexture(3, "greystone.png");
    AddMapTexture(4, "mossy.png");
    AddMapTexture(5, "pillar.png");
    AddMapTexture(69, "greystone.png");*/
//lol this code was from the first evening
}

void CLevelSystem::AddMapTexture(int id, const std::string& name)
{
    auto hTex = ITextureSystem->FindTexture(name);
    if(!ITextureSystem->IsHandleValid(hTex)){
        log("failed to add map texture for mapid %i", id); return;
    }
    auto succ = level_textures.emplace(id, hTex);
    if(!succ.second){
         log("failed to add map texture to db for mapid %i handle %i", id, hTex); return;
    }
}



texture_t* CLevelSystem::GetTextureAt(int x, int y,  uint8_t type) 
{
    auto tile = GetTileAt(x,y);
    if(!type)
        return tile->m_pTexture;
    if(tile->m_nType != Level::Tile_Empty && tile->m_nType == Level::Tile_Wall)
        return tile->m_pTexture;
    
    if(type == TileTexture_Ceiling)
        return tile->m_pTextureCeiling;
    else 
        return tile->m_pTextureFloor;

}

texture_t* CLevelSystem::GetTexturePlane(bool is_floor, int x, int y)
{
    x = std::clamp(x, 0, MAP_SIZE - 1); 
    y = std::clamp(y, 0, MAP_SIZE - 1); 
    auto tile = GetTileAt(x,y);
    if(tile->IsThinWall())
        return (is_floor) ? GetTextureAt(x,y, TileTexture_Floor) : GetTextureAt(x,y, TileTexture_Ceiling);
    if(!is_floor)
        return GetTextureAt(x,y, TileTexture_Ceiling);
    
   //  bool checkerBoardPattern = (int(x + y)) & 1;
    return GetTextureAt(x,y, TileTexture_Floor);
}

void CLevelSystem::AddBulletHole(tile_t* tile, const IVector2 pos, const uint8_t* side, float radius) //How did I break these!?
{
    #define MAX_DECALS 15
 //   log("trying to add bullet hole");
    if(tile->m_nDecals > MAX_DECALS){
         log("> max decals");
        int index = (tile->m_nDecals - 1) % MAX_DECALS;
        auto pDecals = tile->m_pDecals;
        for (int i = 0; i < index; ++i) {
            if(pDecals->m_pNextDecal)
                pDecals = pDecals->m_pNextDecal;
           
            
        }
        if(pDecals == nullptr) return; //always nullptr!!

        pDecals->radius = radius;
        pDecals->m_pNextDecal = nullptr;
        pDecals->texturePosition = pos;
        pDecals->side = side[2];
        pDecals->dir[0] = side[0];
        pDecals->dir[1] = side[1];
         tile->m_nDecals++;
      //    log("> max added");
        return;
    }

    auto hole = new decal_t;
   //  log("made new hole");
    hole->radius = radius;
    hole->m_pNextDecal = nullptr;
    hole->texturePosition = pos;
    hole->side = side[2];
    hole->dir[0] = side[0];
    hole->dir[1] = side[1];

    if(tile->m_nDecals <= 0 || tile->m_pDecals == nullptr){
           tile->m_nDecals = 1;
           tile->m_pDecals = hole;
           return;
    }
    decal_t* pDecals = tile->m_pDecals;
    tile->m_nDecals++;
    int d = 0;
    while (1)
    {
        if(!pDecals){
            log("bullet hole hmm %d ", d);
            delete hole; return;
        }
        if(pDecals->m_pNextDecal == nullptr) break;
        pDecals = pDecals->m_pNextDecal;
         
        d++;
    }
  
    pDecals->m_pNextDecal = hole;
    if(pDecals == nullptr || (pDecals && pDecals->m_pNextDecal == nullptr )){
        log("no hole");
        delete hole; return;
    }

   // log("%i %i", tile->m_nDecals, d);
    pDecals->m_pNextDecal = hole;
}




hTexture CLevelSystem::FindTextureForMapObject(int obj)
{
    hTexture ret;
    try{
        ret =  level_textures.at(obj);
    }
    catch (const std::out_of_range& oor) {
        Error("unset texture for map obj %i", obj);
        
        ret = ITextureSystem->ErrorTextureHandle();
    }
   
   return ret;
}



void CLevelSystem::OnShutdown()
{
}

void CLevelSystem::OnLoopStart()
{
}

void CLevelSystem::OnLoopEnd()
{
}

void CLevelSystem::OnRenderStart()
{
}

void CLevelSystem::OnRenderEnd()
{
}