#pragma once
#include <logger/logger.hpp>
#include <common.hpp>
#include <SDL3/SDL.h>



/*
i didnt realize how easy this was going to be to write so it really shouldnt be its own class but oh well 
maybe it will need platform specific tweaks and i wont regret this

*/

class CImageLoader : private CLogger
{
    public:
        CImageLoader() : CLogger(this) {}

        int Load(const std::string& img_path, SDL_Surface** surf);
    private:
      

};