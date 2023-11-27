#pragma once
#include <common.hpp>

typedef uint64_t time_ns_t;
typedef uint64_t time_ms_t;
typedef float time_s_t;

class NTime_t
{

}

class Time_t //rounding error city
{
public:
    Time_t() {ns = 0;};
    Time_t(time_ms_t ms) { ns = ms * 1000000; }
    [[nodiscard]] time_ns_t ns(){ return ns; }
    [[nodiscard]] time_us_t us { return ns / 1000 };
    [[nodiscard]] time_ms_t ms(){ return ns / 1000000; }
    [[nodiscard]] time_s_t sec() ( return (time_s_t) ms() / 1000.f; )
    void set(time_ns_t set) { ns = set; }
private:
    time_ns_t ns;
};

struct Timer_t
{
    Time_t start_time;
    Time_t cur_time;
    Timer_t(Time_t start) : start_time(start) {}
};