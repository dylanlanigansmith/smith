#include "CPathFinder.hpp"

#include <engine/engine.hpp>

CPathFinder::CPathFinder(CBaseEntity* m_pParent) : CBaseEntityComponent(m_pParent), CLogger(this, m_pParent->GetName() + (std::string)m_iParentID)
{
    Reset();
    Debug(false);
}

std::vector<IVector2> CPathFinder::ReconstructPath()
{
    if(!HasPath() || came_from.empty())
        return std::vector<IVector2>();
    
    std::vector<IVector2> rebuilt_path;

     IVector2 current = vecGoal;
    if (came_from.find(vecGoal) == came_from.end()) {
        return rebuilt_path; // no path can be found
    }
    while (current != vecStart) {
        rebuilt_path.push_back(current);
        current = came_from[current];
    }
    rebuilt_path.push_back(vecStart); // optional
    std::reverse(rebuilt_path.begin(), rebuilt_path.end());

    dbg("made path size = %li, start %s end %s", rebuilt_path.size(), rebuilt_path.front().s(), rebuilt_path.back().s());
    return rebuilt_path;
}


inline double heuristic(IVector2 a, IVector2 b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool CPathFinder::Search(const IVector2 &start, const IVector2 &goal)
{
    
    dbg("searching for path from %s to %s", start.s(), goal.str().c_str());
    Reset();
    vecStart = start; vecGoal = goal;
    if(vecStart != vecGoal){
         m_bReachedGoal = false;
    }
    PriorityQueue<IVector2, double> frontier;
    frontier.put(start, 0);
    came_from[start] = start;
    cost_so_far[start] = 0.0;

    while(!frontier.empty())
    {
        IVector2 current =  frontier.get();
        if(current == goal){
            m_bHasPath = true;

            path = ReconstructPath();
            m_iPathSize = path.size();
            if(path.empty()){
                dbg("path failed "); return false;
            }
            dbg("found path from {%i,%i} to {%i %i}, %li", vecStart.x, vecStart.y, vecGoal.x, vecGoal.y, frontier.elements.size());
            return true;
        }

        for(auto& next : GetNeighbours(current)){
            double new_cost = cost_so_far[current] + 1.0;
            //log("%i %i", next.x, next.y);
            if(cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far.at(next))
            {
                cost_so_far[next] = new_cost;
                double priority = new_cost + heuristic(next, goal);
                frontier.put(next, priority);
                came_from[next] = current;
            }
        }
    }
    return false;
}

std::vector<IVector2> CPathFinder::GetNeighbours(IVector2 pos)
{
    static CLevelSystem* m_levelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    std::vector<IVector2 > ret;
    if(auto t = m_levelSystem->GetTileAt(pos.x, pos.y ); t != nullptr){
        if(t->m_nType == Level::Tile_Empty)
            ret.push_back(t->m_vecPosition);
    }
    if(auto t = m_levelSystem->GetTileAt(pos.x, pos.y + 1); t != nullptr){
        if(t->m_nType == Level::Tile_Empty)
            ret.push_back(t->m_vecPosition);
    }
    if(auto t = m_levelSystem->GetTileAt(pos.x, pos.y - 1); t != nullptr){
        if(t->m_nType == Level::Tile_Empty)
            ret.push_back(t->m_vecPosition);
    }
    if(auto t = m_levelSystem->GetTileAt(pos.x + 1, pos.y); t != nullptr){
        if(t->m_nType == Level::Tile_Empty)
            ret.push_back(t->m_vecPosition);
    }
    if(auto t = m_levelSystem->GetTileAt(pos.x - 1, pos.y); t != nullptr){
        if(t->m_nType == Level::Tile_Empty)
            ret.push_back(t->m_vecPosition);
    }
    return ret;
}

double CPathFinder::FindCost(const IVector2 &old_pos, const IVector2 &new_pos)
{
    static CLevelSystem* m_levelSystem = engine->CreateInterface<CLevelSystem>("ILevelSystem");
    auto tile = m_levelSystem->GetTileAt(new_pos);
    if(!tile)
        return 10.0;

    if(tile->m_occupants.size() > 0 && new_pos != vecGoal)
        return (double)tile->m_occupants.size() * 2.2;

    return 1.0;
}

IVector2 CPathFinder::GetNextPoint(IVector2 pos)
{
    if(!HasPath() || ReachedGoal())
        return pos;

    assert(m_iPathSize == path.size());
    if(m_iPathIndex >= m_iPathSize){
        dbg("we have gone into uncharted waters %li > %li ",m_iPathIndex, m_iPathSize );
        Reset();
        return pos;
    }

    if(pos == vecGoal){
       // Reset();
        m_bReachedGoal = true;
        dbg("reached goal pt {%i %i } in %i moves", pos.x, pos.y, m_iPathIndex);
        return pos;
    }
    else if(m_iPathIndex == m_iPathSize - 1){
         info("reached end of path {%i %i} without hitting goal pt {%i %i } in %li moves", pos.x, pos.y, vecGoal.x, vecGoal.y, m_iPathIndex);
    }
    auto cur = std::find(path.begin(), path.end(), pos);
    if(cur == path.end()){
        log("strayed {%i %i} from the path {%i %i } in %li/%li moves", pos.x, pos.y, path.at(m_iPathIndex).x, path.at(m_iPathIndex).y, m_iPathIndex, m_iPathSize);
        auto prev = path.at(m_iPathIndex);
        Reset();
        return prev;
    }  

    
    if(path.at(m_iPathIndex) == pos && m_iPathIndex + 1 != path.size()){
        m_iPathIndex++;
        dbg("%i %i  {%li/%li}", path.at(m_iPathIndex).x, path.at(m_iPathIndex).y, m_iPathIndex, m_iPathSize );
    }
        

    return path.at(m_iPathIndex);
}
