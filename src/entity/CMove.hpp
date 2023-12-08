#pragma once

#include <types/Vector.hpp>


class CMove
{
public:
    CMove() {
        m_flSpeedModifier = 1.0;
        m_flForwardSpeed = m_flStrafeSpeed = m_flYawSpeed = m_flSpeedModifier;
    }
    double m_flForwardSpeed;
    double m_flStrafeSpeed;
    double m_flYawSpeed;
    double m_flSpeedModifier;
};