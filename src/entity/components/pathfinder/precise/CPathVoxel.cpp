#include "CPathVoxel.hpp"

#include <engine/engine.hpp>

CPathVoxel::CPathVoxel(CBaseEntity* m_pParent) : CBaseEntityComponent(m_pParent), CLogger(this, m_pParent->GetName() + (std::string)m_iParentID)
{
    Reset();
    Debug(true);
}
inline bool closeEnough(const Vector2& cur, const Vector2& goal, double tol = 0.35){
    if( (goal - cur).LengthSqr() <= tol*tol ){
        return true;
    }
    return false;
}
std::vector<Vector2> CPathVoxel::ReconstructPath()
{
    if(!HasPath() || came_from.empty())
        return std::vector<Vector2>();
    
    std::vector<Vector2> rebuilt_path;

     Vector2 current = vecGoal;
      log("{%.3f %.3f} -> {%.3f %.3f}", current.x, current.y, vecGoal.x, vecGoal.y);
     bool found_path = false;
     for(auto& pt : came_from)
     {
        if(closeEnough(pt.first, vecGoal)){
            current = pt.first;
            found_path = true; break;
        }
     }
     
    if (!found_path ) {
        return rebuilt_path; // no path can be found
    }
    while (!closeEnough(current, vecStart, 0.8)) {
        log("{%.3f %.3f} -> {%.3f %.3f}", current.x, current.y, vecStart.x, vecStart.y);
        rebuilt_path.push_back(current);
        bool yeah = false;
        current = came_from[current];
    }
    rebuilt_path.push_back(vecStart); // optional
    std::reverse(rebuilt_path.begin(), rebuilt_path.end());

    dbg("made path size = %li, start  {%.3f %.3f} end  {%.3f %.3f}", rebuilt_path.size(), rebuilt_path.front().x, rebuilt_path.front().y, rebuilt_path.back().x, rebuilt_path.back().y);
    return rebuilt_path;
}


inline double heuristic(Vector2 a, Vector2 b) {
  return (a - b).LengthSqr();
}

bool CPathVoxel::Search(const Vector2 &start, const Vector2 &goal)
{
    
  //  dbg("searching for path from {%.3f %.3f} to {%.3f %.3f}", start.x, start.y, goal.x, goal.y);
    Reset();
    static CLevelSystem* IlevelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto tile_goal = IlevelSystem->GetTileAt(goal);
    auto rg =  tile_goal->getSectorCenterRelativeCoords( tile_goal->worldToSector(goal) );
    vecGoal = {rg.x + floor(goal.x), rg.y + floor(goal.y)};
    auto tile_start = IlevelSystem->GetTileAt(start);
    auto rs =  tile_start->getSectorCenterRelativeCoords( tile_start->worldToSector(start) );
    vecStart = {rs.x + floor(start.x), rs.y + floor(start.y)};

    if(vecStart != vecGoal){
         m_bReachedGoal = false;
    }
    PriorityQueue<Vector2, double> frontier;
    frontier.put(start, 0);
    came_from[start] = start;
    cost_so_far[start] = 0.0;

    while(!frontier.empty())
    {
        Vector2 current =  frontier.get();
        // dbg("looking at {%.3f ,%.3f} to get to {%.3f ,%.3f}, %li", current.x, current.y, vecGoal.x, vecGoal.y, frontier.elements.size());
        if(closeEnough(current, vecGoal)){
            m_bHasPath = true;

            path = ReconstructPath();
            m_iPathSize = path.size();
            if(path.empty()){
                dbg("path failed "); return false;
            }
            dbg("found path from {%.3f ,%.3f} to {%.3f ,%.3f}, %li", vecStart.x, vecStart.y, vecGoal.x, vecGoal.y, frontier.elements.size());
            return true;
        }
       // auto n = GetNeighbours(current);
      //  log("%li neighbors to {%.3f %.3f}", n.size(), current.x, current.y);
        for(auto& next : GetNeighbours(current)){
            double new_cost = cost_so_far[current] + 1.0;
            //log("%i %i", next.x, next.y);
            if(cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far.at(next))
            {
                cost_so_far[next] = new_cost;
                double priority = new_cost + heuristic(next, goal) + FindCost(current, next);
                frontier.put(next, priority);
                came_from[next] = current;
            }
        }
    }
     // dbg("No path from {%.3f ,%.3f} to {%.3f ,%.3f}, %li", vecStart.x, vecStart.y, vecGoal.x, vecGoal.y, frontier.elements.size());
    return false;
}
//
std::vector<Vector2> CPathVoxel::GetNeighbours(const Vector2& pos)
{
    static CLevelSystem* m_levelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    static auto IEntitySystem = engine->CreateInterface<CEntitySystem>("IEntitySystem");
    std::vector<Vector2 > ret;

    auto ptile = m_levelSystem->GetTileAt(pos.x, pos.y );
    auto rel_pos = ptile->worldToSector({pos.x, pos.y, 0});
   const double s = 0.35;
   std::vector<Vector2> nbrs = { {pos.x, pos.y + s}, {pos.x, pos.y - s}, {pos.x + s, pos.y}, {pos.x - s, pos.y}, { pos.x, pos.y }  };
   for(auto& nbr : nbrs ){
        auto tile = m_levelSystem->GetTileAt(nbr.x, nbr.y );
        auto rel  = tile->worldToSector(nbr);
        auto c = tile->getSectorCenterRelativeCoords(rel.x, rel.y, rel.z);
        nbr = {c.x + floor(nbr.x), c.y + floor(nbr.y)};
   }
   for(auto& nbr : nbrs )
   {
        auto tile = m_levelSystem->GetTileAt(nbr.x, nbr.y );
        
        if(!tile) continue;
        if(tile->m_nType == Level::Tile_Wall) continue;

        const double tol = 0.0;/*
        if(nbr.x > (1 - tol))
            if(m_levelSystem->GetTileAt(IVector2::Rounded(nbr.x, floor(nbr.y) ) )->m_nType == Level::Tile_Wall ) continue;
        if(nbr.y > (1 - tol))
            if(m_levelSystem->GetTileAt(IVector2::Rounded(floor(nbr.x), nbr.y ) )->m_nType == Level::Tile_Wall ) continue;
        if(nbr.y < (tol))
            if( auto t = m_levelSystem->GetTileAt(nbr.x, nbr.y - tol ); t && t->m_nType == Level::Tile_Wall ) continue;
        if(nbr.x < (tol))
            if( auto t = m_levelSystem->GetTileAt(nbr.x  - tol, nbr.y ); t && t->m_nType == Level::Tile_Wall ) continue;*/
     
        if(tile->m_nType >= Level::Tile_Door){
            std::array<IVector2, 3> bad_pts;
            switch (tile->m_nType)
            {

            case Level::Tile_WallN:
            bad_pts = { IVector2(0,0), IVector2(1,0), IVector2(2,0),   }; break;
  
            case Level::Tile_WallW:
            bad_pts = { IVector2(2,0), IVector2(2,1), IVector2(2,2),   }; break;
            break;
            case Level::Tile_WallS:
            bad_pts = { IVector2(0,2), IVector2(1,2), IVector2(2,2),   }; break;
            break;
            case Level::Tile_WallE:
            bad_pts = { IVector2(0,0), IVector2(0,1), IVector2(0,2),   }; break;
            break;
            default:
                warn("something changed with thinwalls and I dont like it!");
            };

            auto rel = tile->worldToRelative({nbr.x, nbr.y, 0});
            IVector2 vox_pt = {rel.x, rel.y};
            bool bad = false;
            for(auto& pt : bad_pts)
                if(pt == vox_pt ){
                    bad = true; break;
                }
            if(bad)   continue;
        }
        if(!tile->m_occupants.empty())
        {
            bool bad = false;
            for(auto& occupant : tile->m_occupants)
            {
                if(occupant == m_pParent->GetID()) continue;

                auto ent = IEntitySystem->GetEntity(occupant);
                if(!ent) continue;
                auto bounds = ent->GetBounds();
                Vector2 ent_pos = ent->GetPosition();
                if( (ent_pos - nbr).LengthSqr() <= (bounds * bounds) + tol )
                {
                    bad = true; break;
                }
            }
            if(bad) continue;
        }
      //  if(tile->m_nType == Level::Tile_Empty && tile->m_occupants.empty())
        //    nbr = { floor(nbr.x) + 0.5, floor(nbr.y) + 0.5};

        ret.push_back(nbr);
   }
   
   return ret;
}

double CPathVoxel::FindCost(const Vector2 &old_pos, const Vector2 &new_pos)
{
    static CLevelSystem* m_levelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto tile = m_levelSystem->GetTileAt(new_pos);
    auto old_tile = m_levelSystem->GetTileAt(old_pos);
    if(!tile)
        return 100.0;
    auto rel = tile->worldToSector(new_pos);
    if( (rel.x == 2  || rel.x == 0 )  && rel.y != 1 ) //ideally we see if the edge is along a wall
        return 2.5;
    if( (rel.y == 2  || rel.y == 0 )  && rel.x != 1 )
        return 2.5;
    if(!tile->m_occupants.empty() && tile != old_tile)
        return 1.2 * tile->m_occupants.size();
    if(tile != old_tile)
        return 0.95;    
    return 1.0;
}

Vector2 CPathVoxel::GetNextPoint(Vector2 pos)
{
    if(!HasPath() || ReachedGoal())
        return pos;

    assert(m_iPathSize == path.size());
    if(m_iPathIndex >= m_iPathSize){
        dbg("we have gone into uncharted waters %li > %li ",m_iPathIndex, m_iPathSize );
        Reset();
        return pos;
    }

    if(Vector2::closeEnough(pos, vecGoal)){
       // Reset();
        m_bReachedGoal = true;
        dbg("reached goal pt {%.3f ,%.3f} in %i moves", pos.x, pos.y, m_iPathIndex);
        return pos;
    }
   
    if(m_iPathIndex == m_iPathSize - 1){
         log("reached end of path {%.3f ,%.3f} without hitting goal pt {%.3f ,%.3f} in %li moves", pos.x, pos.y, vecGoal.x, vecGoal.y, m_iPathIndex);
         return vecGoal;
    }
   

    bool close = false;
    for( auto& pt : path)
    {
        if(Vector2::closeEnough(pos, pt)){
            close = true; break;
        }
    }
    if(!close){
        log("strayed {%.3f ,%.3f} from the path {%.3f ,%.3f} in %li/%li moves", pos.x, pos.y, path.at(m_iPathIndex).x, path.at(m_iPathIndex).y, m_iPathIndex, m_iPathSize);
        auto prev = path.at(m_iPathIndex);
        //Reset();
        return prev;
    }  

    
    if(Vector2::closeEnough(pos, path.at(m_iPathIndex) ) ){
        if(m_iPathIndex + 1 < m_iPathSize)
            m_iPathIndex++;
        dbg("{%.3f ,%.3f}  {%li/%li}", path.at(m_iPathIndex).x, path.at(m_iPathIndex).y, m_iPathIndex, m_iPathSize );
    }
        

    return path.at(m_iPathIndex);
}
