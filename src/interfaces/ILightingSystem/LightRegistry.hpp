#pragma once

#include <common.hpp>
#include "ILightingSystem.hpp"

#define REGISTER_DEC_LIGHT(NAME) \
            static RegisterLight<NAME> reg;
                
#define REGISTER_DEF_LIGHT(NAME) \
        RegisterLight<NAME> NAME::reg(#NAME) 
                

template<typename T>
struct RegisterLight { 
    RegisterLight(std::string const& s) { 
        
        CLightingSystem::GetRegister()->insert(std::make_pair(s, &CLightingSystem::AddLightRegisteredType<T>));
    }
};


