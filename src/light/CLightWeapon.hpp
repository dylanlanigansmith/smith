#pragma once
#include <light/CLight.hpp>

#include <interfaces/ILightingSystem/LightRegistry.hpp>
#include <engine/engine.hpp>
class CLightWeapon : public CLight
{
public:
    CLightWeapon() : CLight(this)
    {
        m_nextUpdate = 0;
        m_updateRate = TICKS_PER_S / 2;
        m_trigger = m_lastState = m_state = false;
        m_lastPosition = Vector();
    }
    virtual ~CLightWeapon() {}
    virtual Color CalculateInfluence(const Vector &point, Color &color_in, const light_params &params, const Color &MaxDark)
    {
        Color ret = color_in;
        if (m_state)
        {
            ret = CLight::CalculateInfluenceBase(this, point, color_in, params, MaxDark);
        }
        return ret;
    }

    virtual bool IsStatic() const
    {
        return false;
    }
    virtual bool IsTemporary() const { return true; }
    virtual void OnUpdate()
    {
        auto curTick = IEngineTime->GetCurLoopTick();
        if(curTick > m_nextUpdate && m_state){
           m_trigger = true;
        }
        if (m_trigger)
        {
            m_trigger = false;
            m_state = !m_state;
            if (m_state && (m_vecPosition - m_lastPosition).Length2DSqr() > 0.3 * 0.3)
            {
                UpdateInfluence();
            }
           // engine->log("updating %d [%li] %li cur %li next", m_state, InfluenceSize(), curTick, m_nextUpdate);
            
            ILightingSystem->RegenerateLightingForTempLight(this, !m_state);
           
            //big problemo... we lost prev light values..

            //tiles?
        }
        

        m_lastState = m_state;
    }

    virtual void Flash(uint64_t dur = 8)
    {
        if(!m_state){
            m_nextUpdate = IEngineTime->GetCurLoopTick() + dur;
            m_trigger = true;
        }
        else{
           // m_nextUpdate += dur;
        }
        
    }

    void UpdateInfluence()
    {
        m_lastPosition = m_vecPosition;
        double step = 0.1;
        double radiusSquared = m_flRange * m_flRange;

        Reset();
        //gLog("{%f %f %f}", m_vecPosition.x, m_vecPosition.y, m_vecPosition.z);
        // Calculate bounds for the enclosing cube
        int minX = static_cast<int>((m_vecPosition.x - m_flRange) / step);
        int maxX = static_cast<int>((m_vecPosition.x + m_flRange) / step);
        int minY = static_cast<int>((m_vecPosition.y - m_flRange) / step);
        int maxY = static_cast<int>((m_vecPosition.y + m_flRange) / step);
        minX = std::clamp(minX, 0, (int)(MAP_SIZE / step));
        maxX = std::clamp(maxX, 0, (int)(MAP_SIZE / step));

        minY = std::clamp(minY, 0, (int)(MAP_SIZE / step));
        maxY = std::clamp(maxY, 0, (int)(MAP_SIZE / step));
        for (int i = minX; i <= maxX; ++i)
        {
            for (int j = minY; j <= maxY; ++j)
            {
                for (int k = 0; k < 10; ++k)
                {
                    Vector point = {i * step, j * step, k * step};

                    if ((point - m_vecPosition).Length2D() <= radiusSquared)
                    {
                        m_influence.insert(point);
                      //  gLog("{%f %f %f}", point.x, point.y, point.z);
                    }
                }
            }
        }
    }

protected:
    looptick_t m_nextUpdate;
    looptick_t m_updateRate;
    bool m_state;
    bool m_trigger;
    bool m_lastState;

    Vector m_lastPosition;

private:
    REGISTER_DEC_LIGHT(CLightWeapon);
};