#pragma once
#include <common.hpp>

typedef uint64_t time_ns_t;
typedef uint64_t time_us_t;
typedef uint32_t time_ms_t;
typedef uint64_t looptick_t;
typedef double time_s_t;



class Time_t //rounding error city
{
public:
    Time_t() {_ns = 0;};
    Time_t(time_ns_t ns) : _ns(ns) {}
    Time_t(time_ms_t ms) { _ns = ms * 1000000; }
    [[nodiscard]] time_ns_t ns(){ return _ns; }
    [[nodiscard]] time_us_t us() { return _ns / 1000; };
    [[nodiscard]] time_ms_t ms(){ return _ns / 1000000; }
    [[nodiscard]] time_s_t sec() { return _ns / 1000000000.0f; }
    

    void set(time_ns_t set) { _ns = set; }
    void set_ms(time_ms_t set) { _ns = set * 1000000; }
    Time_t operator-(const Time_t& rhs) const{
        return Time_t(_ns - rhs._ns);
    }
    Time_t operator+(const Time_t& rhs) const{
        return Time_t(_ns + rhs._ns);
    }
    bool operator>(const Time_t& rhs) const{
        return (_ns > rhs._ns);
    }
    bool operator<(const Time_t& rhs) const{
        return (_ns < rhs._ns);
    }
    bool operator>=(const Time_t& rhs) const{
        return (_ns >= rhs._ns);
    }
    bool operator<=(const Time_t& rhs) const{
        return (_ns <= rhs._ns);
    }
    bool operator==(const Time_t& rhs) const{
        return (_ns == rhs._ns); //this will never ever happen?
    }


private:
    time_ns_t _ns;
};

struct Timer_t
{
    Timer_t() { start_time.set(0); cur_time.set(0); }
    Time_t start_time;
    Time_t cur_time;
    Timer_t(Time_t start) : start_time(start), cur_time(start) {}
    Time_t Elapsed() {
        return cur_time - start_time;
    }
    void Reset(Time_t cur){
        start_time = cur_time = cur;
    }
};