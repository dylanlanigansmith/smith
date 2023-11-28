#pragma once
#include <common.hpp>
#include <iostream>

//apologies about this 
//len('/home/dylan/code/smith/') == 23
#define SOURCE_PATH_SIZE 23
#define __FILENAME__ (__FILE__ + SOURCE_PATH_SIZE)

#define Error(str, ...) error(__FILENAME__, __LINE__, str, __VA_ARGS__) 

class CLogger
{
public:
    CLogger() { m_szFmtName = "Log"; formatName();}
    CLogger(const std::string& name) : m_szFmtName(name){ formatName();} 
    virtual ~CLogger(){}
    virtual void dbg(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void log(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void log(const std::string& msg);
    virtual void error(const char* file, int line, const char* fmt, ...) __attribute__((format(printf, 4, 5)));
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
