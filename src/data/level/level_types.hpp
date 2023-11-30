#pragma once
#include <common.hpp>
#include <magic_enum/magic_enum.hpp> //holy s$%# this library is awesome
namespace Level
{
    enum Tile_Type : uint8_t
    {
        Tile_Invalid = 0xFF,
        Tile_Empty = 0,
        Tile_Wall = 1,
        Tile_Opaque = 2,
        Tile_Door = 3,
        Tile_Type_SIZE,
    };

}


