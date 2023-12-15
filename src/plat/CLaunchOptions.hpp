#pragma once
#include <common.hpp>
#include <global.hpp>
#include <SDL3/SDL.h>


class CLaunchOptions
{
public:
    CLaunchOptions() {}

    void Init(int argc, char** argv){
        assert(argc >= 1);
        m_cmd = argv[0];
        if(argc == 1){
            m_hasLaunchOptions = false;
            return;
        }
        m_hasLaunchOptions = true;
        for(int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if(arg.find_first_of("-") != std::string::npos){
                arg = arg.substr(arg.find_first_of("-") + 1);
            } //remove - (ex: -debug )
            m_cmdline.push_back(arg);

        }
    }  
    auto& GetArgList() const { return m_cmdline; }
    auto& GetCmd() const { return m_cmd; }


    bool HasArg(const std::string& name, std::string* arg = nullptr){
        auto search = std::find(m_cmdline.begin(), m_cmdline.end(), name);
        bool ret = search != m_cmdline.end(); 
        if(arg != nullptr ) *arg = *search;
        return ret;
    }
    std::string GetArg(const std::string& name){
        auto search = std::find(m_cmdline.begin(), m_cmdline.end(), name);
        if(search != m_cmdline.end())
            return std::string(*search);
       
        return std::string();
    }
    auto NumArgs() const { return m_cmdline.size() ; }
    private:
    bool m_hasLaunchOptions;

    std::string m_cmd;
    std::vector<std::string> m_cmdline;
};