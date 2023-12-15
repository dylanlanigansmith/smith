#pragma once
#include <logger/logger.hpp>
#include <common.hpp>
#include <SDL3/SDL.h>
#include <types/Vector.hpp>

class CImageLoader : private CLogger
{
    public:
        CImageLoader() : CLogger(this) {}

        int Load(const std::string& img_path, SDL_Surface** surf);
    private:
      

};