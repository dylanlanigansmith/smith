#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include "IEngineTime/IEngineTime.hpp"
#include "IInputSystem/IInputSystem.hpp"
#include "IResourceSystem/IResourceSystem.hpp"
#include "ITextureSystem/ITextureSystem.hpp"
#include "IEntitySystem/IEntitySystem.hpp"
#include "ILevelSystem/ILevelSystem.hpp"
#include "ILightingSystem/ILightingSystem.hpp"
class CInterfaceList 
{
public:
    template <typename T>
    T* AddInterface(){
        T* interface = new T();
        auto base = static_cast<CBaseInterface*>(interface);
        

        auto result = interface_list.emplace(base->name(), interface);
        if(!result.second){
            base->log("failure adding to InterfaceList");
            for(auto i : interface_list){
                base->log(i.first);
            } return (nullptr);
        }
        base->OnCreate();
        base->log("added To Interfacelist");
        return (interface);
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
           // delete interface;
        }
    }
    template <typename T>
    T* CreateInterface(const std::string& name){
        //this should not be used without checking it exists so lets just not

        return static_cast<T*>(interface_list.at(name));
            
    }

    const auto& list() { return interface_list; }
private:
     std::unordered_map<std::string, CBaseInterface*> interface_list;
};