#pragma once
#include <common.hpp>
#include <iostream>



class CLogger
{
public:
    CLogger() { m_szFmtName = "Log"; formatName();}
    CLogger(const std::string& name) : m_szFmtName(name){ formatName();} 
    virtual ~CLogger(){}
    virtual void dbg(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void log(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void log(const std::string& msg);
    
private:
    static std::string _strf(const char* fmt, va_list list);
    static void _logf(const char* fmt, ...) __attribute__((format(printf, 1, 2))); 
    static void _log(std::string msg);

    void formatName();

private:
    
    std::string m_szFmtName;
    uint32_t m_nColor;
    uint8_t m_bPriority;

};