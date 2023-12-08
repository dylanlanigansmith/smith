#pragma once
#include <light/CLight.hpp>

#include <interfaces/ILightingSystem/LightRegistry.hpp>
class CLightOverhead  : public CLight
{
public:
    CLightOverhead() : CLight(this) {}
    virtual ~CLightOverhead() {}

private:
    REGISTER_DEC_LIGHT(CLightOverhead);

};