#pragma once
#include <common.hpp>
#include <global.hpp>
#include <SDL3/SDL.h>
#include <logger/logger.hpp>
#include "platform_util.hpp"
#include "CSystemDialog.hpp"
#include "CLaunchOptions.hpp"
#include "CSystemWindow.hpp"
/*

Tied closely with engine
- handles dir paths and sys info 
*/

struct smith_sys_info
{
    int sys_cores;
    int render_threads_to_use;
    int sys_l1_line_size;
    int sys_ram;
    void find(){
        render_threads_to_use = 0;
        sys_cores = SDL_GetCPUCount();
        sys_l1_line_size = SDL_GetCPUCacheLineSize();
        sys_ram = SDL_GetSystemRAM();
        

        //1 thread for gameloop, 1 thread for audio, N threads for render (need 8 tbh, 500fps mandatory)
        if(sys_cores > 12 && SCREEN_HEIGHT > 360) render_threads_to_use = 10; 
        else if(sys_cores > 10) render_threads_to_use = 8; //likely 12 available 
        else if(sys_cores > 6) render_threads_to_use = 4;
        else if(sys_cores > 2) render_threads_to_use = 2; //all 4 cores a blazin
        else render_threads_to_use = 0;
        //this is arbitrary and likely wrong :)

    }
};



#define PLATFORM CPlatform::Instance()
class CPlatform : private CLogger
{
    friend class CEngine;
    public:
        CPlatform();
        virtual ~CPlatform() {}
        static CPlatform& Instance(){ static CPlatform pf; return pf; }
        bool Init(int argc, char** argv);

        auto GetExecutableCWD() const { return m_basePath; } //Ends With SystemSlash
        char GetFileSystemSlash() { return (m_platType == Platform::WIN) ? '\\' : '/' ;  }
        auto& LaunchOptions() { return m_cmdLine; }
        bool isDeveloperMode() const { return m_devMode;}
        auto GetPlatName() const { return m_platName; }
        auto GetPlatType() const { return m_platType; }
        auto& Dialog() { return m_sysDialog;}
        auto& SysInfo() const { return m_sysInfo; }

        auto& SysWindow() const { return m_window; }

        bool IsLinux() const { return (m_platType == Platform::LINUX); }
        bool IsMacOS() const { return (m_platType == Platform::MACOS); }
        bool IsWindows() const { return (m_platType == Platform::WIN); }
        bool IsIOS() const { return (m_platType == Platform::IOS); }
    private:
        auto& window() { return m_window; }
        bool m_devMode;
        std::string m_platName;
        Platform::Platform_Types m_platType;
        CLaunchOptions m_cmdLine;
        CSystemDialog m_sysDialog;
        CSystemWindow m_window;
        smith_sys_info m_sysInfo;
        std::string m_basePath;


};