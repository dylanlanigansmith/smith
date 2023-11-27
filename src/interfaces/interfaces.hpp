#pragma once
#include <common.hpp>
#include <interfaces/CBaseInterface.hpp>
#include "IEngineTime/IEngineTime.hpp"
#include "IInputSystem/IInputSystem.hpp"


class CInterfaceList 
{
public:
    template <typename T>
    void AddInterface(){
        T* interface = new T();
        auto base = static_cast<CBaseInterface*>(interface);
        base->OnCreate();

        auto result = interface_list.emplace(base->name(), interface);
        if(!result.second){
            base->log("failure adding to InterfaceList");
            for(auto i : interface_list){
                base->log(i.first);
            } return;
        }
        base->log("added To Interfacelist");
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
    std::unique_ptr<T> CreateInterface(const std::string& name){
        //this should not be used without checking it exists so lets just not

        return std::unique_ptr<T>(
            static_cast<T*>(interface_list.at(name)));
    }

    const auto& list() { return interface_list; }
private:
     std::unordered_map<std::string, CBaseInterface*> interface_list;
};