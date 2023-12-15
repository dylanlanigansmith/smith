#pragma once
#include <common.hpp>
#include <SDL3/SDL.h>

class CSystemDialog
{
public: 
    CSystemDialog() : m_SDLWindow(NULL) {}
    bool MessageBox(const char* fmt, ... ){ __attribute__((format(printf, 2, 3))) //defaults to fatal error
        va_list args;
        va_start(args, fmt);
        bool ret = TypedFormatMessage(fmt, args);
        va_end(args);
        return ret;
    }
    bool MessageBox(int type, const char* fmt, ... ){ __attribute__((format(printf, 3, 4))) //defaults to fatal error
        va_list args;
        va_start(args, fmt);
        bool ret = TypedFormatMessage(fmt, args, type);
        va_end(args);
        return ret;
    }
    bool MessageBox(std::string msg, int type = SDL_MESSAGEBOX_ERROR)
    {
        if(SDL_ShowSimpleMessageBox(type, GetTitleForType(type), msg.c_str(), m_SDLWindow) != 0){
            m_error = SDL_GetError(); return false;
        }
        return true;
    }
    void SetHostWindow(SDL_Window* win){ m_SDLWindow = win; }
    auto GetError() const { return m_error; }
private:
    SDL_Window* m_SDLWindow;
    std::string m_error;
    bool TypedFormatMessage(const char *fmt, va_list list,  int type = SDL_MESSAGEBOX_ERROR){
        std::string str = FormatMessage(fmt, list);
        return MessageBox(str, type);
    }
    std::string FormatMessage(const char* fmt, va_list list){
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, list);
        return std::string(buf);
    }

    const char* GetTitleForType(int type){
        const char* error_title   = "SmithEngine- Fatal Error";
        const char* warning_title = "SmithEngine - Warning!";
        const char* message_title = "SmithEngine - Confirm: ";
        const char* title = "SmithEngine";
        switch(type)
        {
            case SDL_MESSAGEBOX_ERROR:
                return error_title;
            case SDL_MESSAGEBOX_WARNING:
                return warning_title;
            case SDL_MESSAGEBOX_INFORMATION:
                return message_title;
            default:
                return title;
        }
    }
};
