#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include "IEngineTime/IEngineTime.hpp"



class CInterfaceList 
{
public:
    template <typename T>
    void AddInterface(){
        T* interface = new T();
        auto base = (CBaseInterface*)(interface);
        base->OnCreate();
        base->log("added To Interfacelist");
        interface_list.emplace(base->name(), interface);
    }
    [[nodiscard]] bool InterfaceExists(const std::string& name){
        auto find = interface_list.find(name);
        return (find != interface_list.end());
    }

    void Destroy()
    {
        for(auto& entry : interface_list)
        {
            auto interface = entry.second;
            interface->OnShutdown();
            delete interface;
        }
    }
    template <typename T>
    std::unique_ptr<T> CreateInterface(const std::string& name){
        //this should not be used without checking it exists so lets just not

        return std::unique_ptr<T>(
            static_cast<T>(interface_list.at(name)));
    }

    const auto& list() { return interface_list; }
private:
     std::unordered_map<std::string, CBaseInterface*> interface_list;
};