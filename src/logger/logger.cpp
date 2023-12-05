#include "logger.hpp"
#include <stdio.h>
#include <util/misc.hpp>
#include <global.hpp>
#include <ctime>


bool CLogger::gLogToFile = LOG_FILEGLOBAL;
std::string CLogger::gLogFilePath = LOG_HOME_PATH + LOG_RESOURCE_PATH + LOG_SUBDIR;

void CLogger::formatName()
{
    m_szFmtName.append(" > ");
    m_bDebug = true;
}

inline void CLogger::_instance_log(const std::string &msg)
{
    if(!m_bFileLogging && !m_bColorizeEverything)
        _log(msg);

    if(shouldLogFile())
        _instance_logfile_(msg);
    if(m_bOnlyFileLogging) return;

    if(m_bColorizeEverything)
        _logclr(msg, m_iColorizeColor);
    else  _log(msg);
}

void CLogger::_instance_logfile_(const std::string &msg)
{
    m_fsLogFile << msg << std::endl; //auto flushes 
}

CLogger::CLogger(const std::string& classname, const std::string& name)
{
    m_szFmtName = classname + "::" + name;
    formatName();
}

CLogger::~CLogger()
{
    if(shouldLogFile()){
        log("closing log file: " + m_szFilePath);
        m_fsLogFile.close();
    }
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
    _instance_log(ctxt);
}

void CLogger::log(const char *fmt, ...)
{
    
    std::string ctxt = m_szFmtName;
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    ctxt.append(sfmt);
    
    _instance_log(ctxt);
    
}

void CLogger::log(const std::string &msg)
{
    std::string ctxt = m_szFmtName;
    ctxt.append(msg);
    _instance_log(ctxt);
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

void CLogger::warn(const char *fmt, ...)
{
    std::string ctxt = m_szFmtName;
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    ctxt.append(sfmt);
    _logclr(ctxt, 33);
}

void CLogger::warn(const std::string &msg)
{
    _logclr(msg, 33);
}

void CLogger::status(const char *fmt, ...)
{
    std::string ctxt = m_szFmtName;
    va_list args;
    va_start(args, fmt);
    std::string sfmt = _strf(fmt, args);
    va_end(args);
    ctxt.append(sfmt);
    _logclr(ctxt, 32);
}

void CLogger::status(const std::string &msg)
{
    _logclr(msg, 32);
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

bool CLogger::StartLogFileForInstance(const std::string &path)
{
    m_szFilePath = gLogFilePath + path;
    m_fsLogFile.open(path);
    if(m_fsLogFile.fail()){
        Error("failed to open logfile at %s", m_szFilePath.c_str()); return false;
    }
    m_bFileLogging = true;
    status("started logging to file://" + m_szFilePath);
     _instance_logfile_(_timestr(true));

    return true;
}

bool CLogger::EndLogFileForInstance()
{
    if(!shouldLogFile()){
        Error("attempting to end logfile [%s] that was never started",  m_szFilePath.c_str()); return false;
    }
    _instance_logfile_(_timestr(false));
    m_bFileLogging = false;
    m_fsLogFile.close();
    if(m_fsLogFile.fail()){
        Error("failed to close logfile at %s", m_szFilePath.c_str()); return false;
    }
    warn("stopped logging to file://" + m_szFilePath);
    

    return true;
}

std::string CLogger::_strf(const char *fmt, va_list list)
{
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, list);
    return std::string(buf);
}

std::string CLogger::_timestr(bool full)
{
    std::time_t t = std::time(0);
   
    if(full)
        return std::string(ctime(&t));
    std::tm* now = std::localtime(&t);

    return Util::stringf("%i:%i:%i", now->tm_hour, now->tm_min, now->tm_sec);
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
    std::string clrs = "\033[1;31m";
    switch(clr){
        case 32:
            clrs = "\033[1;32m"; break;
        case 33:
            clrs = "\033[1;33m"; break;
        case 34:
            clrs = "\033[1;34m"; break;
        case 31:
        default:
            clrs =  "\033[1;31m";
    }

    std::cout << clrs << msg << "\033[0m" << std::endl; 
}

std::vector<std::string> CLogger::history = {std::string(__TIME__), std::string(__DATE__)};

void CLogger::_logfile(const std::string msg)
{
   history.push_back(msg);
}