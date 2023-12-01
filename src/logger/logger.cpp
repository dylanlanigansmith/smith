#include "logger.hpp"
#include <stdio.h>

void CLogger::formatName()
{
    m_szFmtName.append(" > ");
    m_bDebug = true;
}

CLogger::CLogger(const std::string& classname, const std::string& name)
{
    m_szFmtName = classname + "::" + name;
    formatName();
}

void CLogger::dbg(const char *fmt, ...)
{
    if(!m_bDebug)
        return;
    std::string ctxt = m_szFmtName;
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    ctxt.append(sfmt);
    _log(ctxt);
}

void CLogger::log(const char *fmt, ...)
{
    
    std::string ctxt = m_szFmtName;
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    ctxt.append(sfmt);
    
    _log(ctxt);
    
}

void CLogger::log(const std::string &msg)
{
    std::string ctxt = m_szFmtName;
    ctxt.append(msg);
    _log(ctxt);
}

void CLogger::info(const char *fmt, ...)
{
    
    std::string ctxt = m_szFmtName;
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    ctxt.append(sfmt);
    
    _logfile(ctxt);
    
}

void CLogger::info(const std::string &msg)
{
    std::string ctxt = m_szFmtName;
    ctxt.append(msg);
    _logfile(ctxt);
}

void CLogger::error(const char* file, int line, const char* fmt, ...) 
{
     std::string ctxt = m_szFmtName;
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    char buf[512];
    sprintf(buf, " %s @ (%i) !> ", file, line);
    ctxt.append(buf);
    ctxt.append(sfmt);
    
    _log(ctxt);
}


std::string CLogger::_strf(const char *fmt, va_list list)
{
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, list);
    return std::string(buf);
}

void CLogger::_logf(const char *fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    std::string s = buf;
    _log(s);
}

void CLogger::_log(std::string msg)
{
        std::cout << msg << std::endl; 
}

std::vector<std::string> CLogger::history = {std::string(__TIME__), std::string(__DATE__)};

void CLogger::_logfile(const std::string msg)
{
   history.push_back(msg);
}