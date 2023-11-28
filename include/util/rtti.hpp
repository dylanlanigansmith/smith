#pragma once
#include <common.hpp>
#include <type_traits>
namespace Util
{
    template <typename T> std::string getClassName(T*){
        
        int status;
        char* demangled = abi::__cxa_demangle(typeid(T).name(), 0,0, &status);
        std::string ret(demangled);
        free(demangled);
        return ret;
    }
}