#pragma once

#include <cstdio>
//https://stackoverflow.com/questions/66126750/trim-both-path-and-extension-from-file-compile-time
constexpr unsigned long filename_we_size(const char *path) {
    // I know - some C pointer stuff. I don't know which C++ functions are 
    // constexpr which are not, and I am too lazy to check, so I used C pointers. 
    // Preferably rewrite it in more C++-ish style.
    auto i = path;
    while (*i) ++i;
    auto end = i;
    while (*i != '.' && i != path) --i;
    const auto ext_len = end - i;
    while (*i != '/' && i != path) --i;
    const auto filename_len = end - i;
    return filename_len - ext_len;
}

constexpr const char *filename_we(const char *path, char *out) {
    auto i = path;
    while (*i) ++i;
    while (*i != '/' && i != path) --i;
    if (*i) ++i;
    auto r = out;
    while (*i != '.' && *i) *r++ = *i++;
    *r = 0;
    return r;
}

// A structure. *Something* has to store the memory.
template <size_t N>
struct Tag {
    char mem[N]{};
    constexpr Tag(const char *path) {
        filename_we(path, mem);
    }
    constexpr const char *str() const {
        return mem;
    }
    constexpr operator const char *() const{
        return mem;
    }    
    constexpr char operator[](size_t i) const {
        return mem[i];
    }
};

//static constexpr Tag<filename_we_size(__FILE__)> TAG{__FILE__} Tag.str();

constexpr const char* filename(std::string_view path) {
    return path.substr(path.find_last_of('/') + 1).data();
}

#define __CEFN_NAME(a ,b) a##b
#define __CEFN(a ,b) __CEFN_NAME(a,b)
#define CE_FILE(val) 