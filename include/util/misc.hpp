#include <common.hpp>
#include <stdio.h>
namespace Util
{
    static const std::string stringf(const char *fmt, ...)
    { 
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        return std::string(buf);
    }
}