#pragma once
#include <light/CLight.hpp>

#include <interfaces/ILightingSystem/LightRegistry.hpp>
#include <engine/engine.hpp>
class CLightDynamic  : public CLight
{
public:
    CLightDynamic() : CLight(this) {
        m_style = 0; m_lastUpdate = 0; m_updateRate = TICKS_PER_S + 9;
        m_state = 0;
    }
    virtual ~CLightDynamic() {}
    virtual Color CalculateInfluence(const Vector& point, Color& color_in, const light_params& params, const Color& MaxDark)
    {
        Color ret = MaxDark; 
        if(m_state == false){
            auto oldRange = GetRange();
            auto oldIntensity = GetIntensity();
            ret = color_in;
        }
        else{

            ret = CLight::CalculateInfluenceBase(this, point, color_in, params, MaxDark);
        }

        return ret;
        
    }

    virtual bool IsStatic() const {
        return false;
    }

    virtual void OnUpdate()
    {
        auto curTick = IEngineTime->GetCurLoopTick();

        if(curTick > m_lastUpdate + m_updateRate)
        {
            m_updateRate = Util::SemiRandRange(TICKS_PER_S / 2, TICKS_PER_S * 2);
            if(m_style == 1){}
            else
            {
                m_state = (m_state > 0) ? 0 : 1; 
                ILightingSystem->RegenerateLightingForDynamicLight(this);

            }
            m_lastUpdate = curTick;
        }
    }   
protected:
    looptick_t m_lastUpdate;
    looptick_t m_updateRate;
    int m_style; 
    int m_state;
private:
    REGISTER_DEC_LIGHT(CLightDynamic);

};