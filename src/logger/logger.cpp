#include "logger.hpp"
#include <stdio.h>
#include <util/misc.hpp>
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
     std::string filename = Util::stripPath(file);
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

void CLogger::_logclr(std::string msg, uint8_t clr)
{

    /* https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
    cout << "\033[1;31mbold red text\033[0m\n";
Here, \033 is the ESC character, ASCII 27. It is followed by [, then zero or more numbers separated by ;, and finally the letter m. The numbers describe the colour and format to switch to from that point onwards.
             foreground background
black        30         40
red          31         41
green        32         42
yellow       33         43
blue         34         44
magenta      35         45
cyan         36         46
white        37         47

reset             0  (everything back to normal)
bold/bright       1  (often a brighter shade of the same colour)
underline         4
inverse           7  (swap foreground and background colours)
bold/bright off  21
underline off    24
inverse off      27
    
    
    
    
    */
        std::cout << "\033[1;31m" << msg << "\033[0m" << std::endl; 
}

std::vector<std::string> CLogger::history = {std::string(__TIME__), std::string(__DATE__)};

void CLogger::_logfile(const std::string msg)
{
   history.push_back(msg);
}