#pragma once
#include <cstdint>

namespace Util
{
    namespace fnv1a {
       
        constexpr uint64_t val_64_const = 14695981039346656037;
        constexpr uint64_t prime_64_const = 1099511628211;


        /*
              prime: 2^40 + 2^8 + 0xb3 = 1099511628211
             offset: 14695981039346656037
        */
        constexpr uint32_t val_32_const = 0x811c9dc5;
        constexpr uint32_t prime_32_const = 0x1000193;

        constexpr uint32_t Hash(const char* str) noexcept { return (*str ? (Hash(str + 1) ^ *str) * prime_32_const : val_32_const); }
        constexpr uint32_t Hash64(const char* str) noexcept { return (*str ? (Hash64(str + 1) ^ *str) * prime_64_const : val_64_const); }

        inline constexpr uint32_t hash_32_fnv1a_const(const char* const str, const uint32_t value = val_32_const) noexcept {
            return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
        }
        inline constexpr uint64_t hash_64_fnv1a_const(const char* const str, const uint64_t value = val_64_const) noexcept {
            return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
        }
    }
}

