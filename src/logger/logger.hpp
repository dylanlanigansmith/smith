#pragma once
#include <common.hpp>
#include <iostream>
#include <util/rtti.hpp>
#include <fstream>
#include <util/constexpr_file.hpp>
//apologies about this 
//len('/home/dylan/code/smith/') == 23
#define SOURCE_PATH_SIZE 23
#define __FILENAME__ (__FILE__ + SOURCE_PATH_SIZE)



#define Error(str, ...) error(__FILE__, __LINE__, str, __VA_ARGS__) 
#define gError(str, ...) CLogger::TypedError(this, __FILE__, __LINE__, str, __VA_ARGS__)
#define gLog(str, ...) CLogger::TypedLog(this, str, __VA_ARGS__)


class CLogger
{
public:
    CLogger() { m_szFmtName = "Log"; formatName();}
    CLogger(const std::string& name) : m_szFmtName(name){ formatName();} 

    template <typename T> 
    CLogger(T* owner, const std::string& name) : CLogger(Util::getClassName<T>(owner), name) {}
    template <typename T> 
    CLogger(T* owner) : CLogger(Util::getClassName<T>(owner)) {}
    CLogger(const std::string& classname, const std::string& name);

    virtual ~CLogger();
    virtual void dbg(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void log(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void log(const std::string& msg);
    virtual void info(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void info(const std::string& msg);
    virtual void note(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void note(const std::string& msg);
    virtual void warn(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void warn(const std::string& msg) ;
    virtual void status(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    virtual void status(const std::string& msg);
    virtual void error(const char* file, int line, const char* fmt, ...) __attribute__((format(printf, 4, 5)));
    virtual bool Debug() const { return m_bDebug; }
    virtual bool Debug(bool set)  { m_bDebug = set; return m_bDebug; }

    virtual bool StartLogFileForInstance(const std::string& path, bool unique = true);
    virtual bool EndLogFileForInstance();
    virtual void SetLogFileOnly(bool set) { m_bOnlyFileLogging = set; status("set onlyFileLogging: %i", m_bOnlyFileLogging); }
    virtual void SetLogColor(int clr) {m_bColorizeEverything = (clr != -1); m_iColorizeColor = clr; } //-1 to disable, clr is ascii terminal

    inline std::string GetTime(bool full = false){ return _timestr(full); }

    static CLogger& Instance() { static CLogger glog("Global"); return glog; }
    template <typename T> 
    static void TypedError(T* thrower, const char* file, int line, const char* fmt, ...) __attribute__((format(printf, 4, 5)));
     template <typename T> 
    static void TypedLog(T* thrower, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
private:
    inline static std::string _strf(const char* fmt, va_list list);
    inline static std::string _timestr(bool full = false);
    static void _logf(const char* fmt, ...) __attribute__((format(printf, 1, 2))); 
    inline static void _log(std::string msg);
    inline static void _logfile(const std::string msg);
    inline static void _logclr(std::string msg, uint8_t clr = 31);
    static std::vector<std::string> history;
    void formatName();
    static std::string formatName(std::string& name);
    static bool gLogToFile;
    static std::string gLogFilePath;
    static std::ofstream& __fs() { if(!gLogToFile) _logclr("Accessing gLogStream with gLogTofile Off!!!"); static std::ofstream f; return f;  }
private:
    inline bool shouldLogFile() { return m_bFileLogging && m_fsLogFile.is_open(); }
    inline void _instance_log(const std::string& msg);
    inline void _instance_logfile_(const std::string& msg);
    inline void _instance_logothers(const std::string& msg); //basically wrapper for logfile with a check for if we are file logging
   

    static inline std::string _stripPath(const std::string& path){
         return path.substr(path.find_last_of("/") + 1);
    }
private:
    bool m_bColorizeEverything = false;
    int m_iColorizeColor = -1;
    bool m_bDebug = true;
    std::string m_szFmtName;
    uint32_t m_nColor;
    uint8_t m_bPriority;

    bool m_bFileLogging;
    bool m_bOnlyFileLogging;
    std::string m_szFilePath;
    std::ofstream m_fsLogFile;
};


inline std::string CLogger::formatName(std::string& name)
{
    return name.append(" > ");
}
template <typename T>
inline void CLogger::TypedError(T *thrower, const char *file, int line, const char *fmt, ...)
{
    std::string ctxt =  Util::getClassName<T>(thrower);
    formatName(ctxt);
    std::string filename = _stripPath(file);
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    char buf[1024];
    sprintf(buf, " %s @ (%i) !> ", filename.c_str(), line);
    ctxt.append(buf);
    ctxt.append(sfmt);
    
    _logclr(ctxt);
    
}

template <typename T>
inline void CLogger::TypedLog(T *thrower, const char *fmt, ...)
{
    std::string ctxt =  Util::getClassName<T>(thrower) ;
      formatName(ctxt);
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    ctxt.append(sfmt);
    
    _logclr(ctxt, 34);
    
}
