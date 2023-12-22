#pragma once
#include <common.hpp>
#include <queue>
#include <data/level.hpp>
#include <types/Vector.hpp>
#include <interfaces/ILevelSystem/ILevelSystem.hpp>
#include <logger/logger.hpp>
#include <entity/components/CBaseEntityComponent.hpp>
#include <entity/components/pathfinder/CPathFinder.hpp>
//creds here for this amazing resource
//https://www.redblobgames.com/pathfinding/a-star/introduction.html
//one of the best programming articles I have ever found


class CPathVoxel : public CBaseEntityComponent
{


  friend class CEditor;
public:
    CPathVoxel(CBaseEntity* m_pParent);
    virtual ~CPathVoxel(){}
    virtual void OnCreate() {}
    virtual void OnUpdate() {}
    bool Search(const Vector2& start, const Vector2& goal);

    [[nodiscard]] bool HasPath() const { return m_bHasPath; }
    [[nodiscard]] bool ReachedGoal() const { return m_bReachedGoal; }
    virtual Vector2 GetNextPoint(Vector2 pos);

    virtual void Reset() {
        m_iPathSize = m_iPathIndex = 0;
        m_bHasPath  = false; //maybe reset reachedGoal when a new goal is set
        vecGoal = {};
        vecStart = {};
        came_from.clear();
        cost_so_far.clear();
        path.clear();
    }
    auto& GetGoal() const { return vecGoal; }
protected:
    std::vector<Vector2> ReconstructPath();
    std::vector<Vector2> GetNeighbours(const Vector2& pt);

    virtual double FindCost(const Vector2& old_pos, const Vector2& new_pos);

protected:
    std::size_t m_iPathIndex;
    std::size_t m_iPathSize;

    bool m_bHasPath;
    bool m_bReachedGoal;
    Vector2 vecStart;
    Vector2 vecGoal;
    std::unordered_map<Vector2, Vector2> came_from;
    std::unordered_map<Vector2, double> cost_so_far;

    std::vector<Vector2> path;
    

};

