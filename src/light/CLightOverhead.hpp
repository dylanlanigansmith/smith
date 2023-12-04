#pragma once
#include <light/CLight.hpp>
class CLightOverhead  : public CLight
{
public:
    CLightOverhead() : CLight(this) {}
    virtual ~CLightOverhead() {}

};