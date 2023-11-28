#pragma once
#include <common.hpp>

namespace Murmur2{
//dont use this for now it's copied straight from valve source2 code
uint64_t static CreateHash(const char* str){
    const uint32_t m = 0x5bd1e995;
    const int r = 24;
    const uint32_t seed = 0x31415926;

    int len = strlen(str); //zero terminated
    uint32_t hash = seed ^ len;
    const unsigned char *data = (const unsigned char *)str;

    while (len >= 4) {
        uint32_t k = *(uint32_t *)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        hash *= m;
        hash ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
        case 3:
            hash ^= data[2] << 16;
            // fall through
        case 2:
            hash ^= data[1] << 8;
            // fall through
        case 1:
            hash ^= data[0];
            hash *= m;
    };

    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;

    return (uint64_t)hash;

}
}