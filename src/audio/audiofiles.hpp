#pragma once 
#include <common.hpp>

namespace AudioImport
{
    void LoadWAV(const std::string& path, uint8_t** buf, int* len);
}