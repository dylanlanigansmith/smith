#pragma once 
#include <common.hpp>
#include "audio_types.hpp"
#include <SDL3/SDL_audio.h>
namespace AudioImport
{
    int LoadWAV(const std::string& path, uint8_t** buf, uint32_t* len, SDL_AudioSpec* spec);
    void LoadOGG(const std::string& name, audiodata_t* ad);
    audiodata_t* Load(const std::string& name_ext, SDL_AudioSpec* spec);
    std::string GetPathToAudio(const std::string& name);
}