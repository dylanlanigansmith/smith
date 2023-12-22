#pragma once
#include <common.hpp>
#include <queue>
#include <data/level.hpp>
#include <types/Vector.hpp>
#include <interfaces/ILevelSystem/ILevelSystem.hpp>
#include <logger/logger.hpp>
#include <entity/components/CBaseEntityComponent.hpp>
//creds here for this amazing resource
//https://www.redblobgames.com/pathfinding/a-star/introduction.html
//one of the best programming articles I have ever found


class CPathFinder : public CBaseEntityComponent
{
  friend class CEditor;
public:
    CPathFinder(CBaseEntity* m_pParent);
    virtual ~CPathFinder(){}
    virtual void OnCreate() {}
    virtual void OnUpdate() {}
    bool Search(const IVector2& start, const IVector2& goal);

    [[nodiscard]] bool HasPath() const { return m_bHasPath; }
    [[nodiscard]] bool ReachedGoal() const { return m_bReachedGoal; }
    virtual IVector2 GetNextPoint(IVector2 pos);

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
    auto& GetStart() const { return vecStart; }

    auto HalfwayToGoal() const { return m_iPathIndex >= (m_iPathSize / 2); }
protected:
    std::vector<IVector2> ReconstructPath();
    std::vector<IVector2> GetNeighbours(IVector2 pt);

    virtual double FindCost(const IVector2& old_pos, const IVector2& new_pos);

protected:
    std::size_t m_iPathIndex;
    std::size_t m_iPathSize;

    bool m_bHasPath;
    bool m_bReachedGoal;
    IVector2 vecStart;
    IVector2 vecGoal;
    std::unordered_map<IVector2, IVector2> came_from;
    std::unordered_map<IVector2, double> cost_so_far;

    std::vector<IVector2> path;
    

};

//https://www.redblobgames.com/pathfinding/a-star/implementation.html#algorithm
template<typename T, typename priority_t>
struct PriorityQueue {
  typedef std::pair<priority_t, T> PQElement;
  std::priority_queue<PQElement, std::vector<PQElement>,
                 std::greater<PQElement>> elements;

  inline bool empty() const {
     return elements.empty();
  }

  inline void put(T item, priority_t priority) {
    elements.emplace(priority, item);
  }

  T get() {
    T best_item = elements.top().second;
    elements.pop();
    return best_item;
  }
};