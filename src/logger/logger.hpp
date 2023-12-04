#pragma once
#include <common.hpp>
#include <iostream>
#include <util/rtti.hpp>

#include <util/constexpr_file.hpp>
//apologies about this 
//len('/home/dylan/code/smith/') == 23
#define SOURCE_PATH_SIZE 23
#define __FILENAME__ (__FILE__ + SOURCE_PATH_SIZE)



#define Error(str, ...) error(__FILE__, __LINE__, str, __VA_ARGS__) 

class CLogger
{
public:
    CLogger() { m_szFmtName = "Log"; formatName();}
    CLogger(const std::string& name) : m_szFmtName(name){ formatName();} 

    template <typename T> 
    CLogger(T* owner, const std::string& name) : CLogger(Util::getClassName<T>(owner), name) {}
    CLogger(const std::string& classname, const std::string& name);

    virtual ~CLogger(){}
    virtual void dbg(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void log(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void log(const std::string& msg);
    virtual void info(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void info(const std::string& msg);
    virtual void error(const char* file, int line, const char* fmt, ...) __attribute__((format(printf, 4, 5)));
    virtual bool Debug() const { return m_bDebug; }
    virtual bool Debug(bool set)  { m_bDebug = set; return m_bDebug; }
private:
    bool m_bDebug = true;
    static std::string _strf(const char* fmt, va_list list);
    static void _logf(const char* fmt, ...) __attribute__((format(printf, 1, 2))); 
    static void _log(std::string msg);
    static void _logfile(const std::string msg);
    static void _logclr(std::string msg, uint8_t clr = 0);
    static std::vector<std::string> history;
    void formatName();

private:
    
    std::string m_szFmtName;
    uint32_t m_nColor;
    uint8_t m_bPriority;

};
