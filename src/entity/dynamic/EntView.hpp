#pragma once 

#include <types/Vector.hpp>


struct EntView
{
    Vector2 m_dir;

    EntView() : m_dir(-1, 0) {}
    

    void rotate(double deg){ m_dir.rotate(deg); }
    void lookAt(const Vector2& pos, const Vector2& target){
        auto delta = target - pos;
        m_dir = delta.Normalize();
    }
    bool lookAt(const Vector2& pos, const Vector2& target, double speed, double close_enough = 4.0){
        Vector2 targetDirection = (target - pos).Normalize();
        if((target - pos).Length() < 0.04)
            return true;
        auto newDir = m_dir.lerp(targetDirection, speed);
        if(std::isnan(newDir.x) || std::isnan(newDir.y))
            return true;
        m_dir = newDir;
         double dot = m_dir.dotClamped(targetDirection);
       
        double angle = std::acos(dot) * RAD2DEGNUM; 

        // Check if the angle is within the threshold
        return angle <= close_enough;

    }
};
