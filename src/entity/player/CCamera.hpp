#pragma once
#include <types/Vector.hpp>

class CCamera
{
public:
    Vector m_vecPosition;
    Vector2 m_vecDir;
    Vector2 m_vecPlane;
    double m_flPitch;
};