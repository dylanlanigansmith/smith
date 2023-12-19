#pragma once



#include <common.hpp>
class CEntitySystem;
#include "IEntitySystem.hpp"


#define REGISTER_DEC_ENT(NAME) \
            static RegisterEntity<NAME> reg;
                
#define REGISTER_DEF_ENT(NAME) \
        RegisterEntity<NAME> NAME::reg(#NAME) 
                

template<typename T>
struct RegisterEntity { 

   
    RegisterEntity(std::string const &s)
    {
        CEntitySystem::GetRegistry()->insert(std::make_pair(s, &CEntitySystem::AddEntByRegistry<T>));  
    }

};


