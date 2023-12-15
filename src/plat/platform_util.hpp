#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_revision.h>
#include <util/misc.hpp>


namespace Platform
{
    enum  Platform_Types : int
    {
        LINUX = 0,
        MACOS,
        WIN,
        IOS,
        UNKNOWN
    };

    inline std::string GetPlatformName()
    {
        return std::string(SDL_GetPlatform());
    }

     inline Platform_Types GetPlatformTypeByName(const char* name = nullptr){
        if(name == nullptr)
            name = SDL_GetPlatform();
        
        if(strcmp(name, "Linux") == 0){
            return LINUX;
        }
        if(strcmp(name, "macOS") == 0){
            return MACOS;
        }
        if(strcmp(name, "Windows") == 0){
            return WIN;
        }
        if(strcmp(name, "iOS") == 0){
            return IOS;
        }
        return UNKNOWN;
    }
     inline bool CheckSDLVersion(std::string& error)
    {
        SDL_version compiled, linked;
        const char* revision = SDL_REVISION;
        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        if(compiled.major != linked.major || compiled.minor != linked.minor || compiled.patch != linked.patch){
            error = Util::stringf("SDL Version Mismatch: Compiled with: %u.%u.%u, linked with %u.%u.%u! Expected rev/ %s, got %s", 
                compiled.major, compiled.minor, compiled.patch, linked.major, linked.minor, linked.patch, revision, SDL_GetRevision() );
            return false;
        }
        return true;
    }


    inline std::string GetBasePath(){
        const char* path = SDL_GetBasePath();

        if(path == NULL)
            return std::string();
        std::string base = path;
        SDL_free((void*)path);
        return base;
    }
}