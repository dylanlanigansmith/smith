#pragma once
#include <SDL3/SDL_pixels.h>

#include <cstdio>
#include <string>




class Color
{
public:
    constexpr Color() : m_uColor(0) {}
              
    constexpr Color(uint32_t m_uColor) : m_uColor(m_uColor) {}
    constexpr  Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255  )
    {  m_uColor = (r << 24) | (g << 16) | (b << 8) | a; }

    constexpr Color(float x, float y, float z, float w, uint8_t s) : Color( (uint8_t)( x * s), (uint8_t)( y * s), (uint8_t)( z * s), (uint8_t)( w *s) )  {    }
    Color(SDL_Color color) { m_uColor = (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a; }

    constexpr inline uint8_t r() const { return ((m_uColor >> 24) & 0xFF);   } 
    constexpr inline uint8_t g() const { return ((m_uColor >> 16) & 0xFF);   } 
    constexpr inline uint8_t b() const { return ((m_uColor >> 8) & 0xFF);   } 
    constexpr inline uint8_t a() const { return ((m_uColor) & 0xFF);   } 

    inline void r(uint8_t v) { m_uColor = (m_uColor & 0x00FFFFFF) | (v << 24); }
    inline void g(uint8_t v) { m_uColor = (m_uColor & 0xFF00FFFF) | (v << 16); }
    inline void b(uint8_t v) { m_uColor = (m_uColor & 0xFFFF00FF) | (v << 8); }
    inline void a(uint8_t v) { m_uColor = (m_uColor & 0xFFFFFF00) | v; }

    inline Color Opaque() { return Color(m_uColor | 0xFF); }
    inline auto s() const { return (std::string)*this; } 
    inline operator SDL_Color() const {
        return SDL_Color{ r() ,g(),b(),a()};
    }
    inline operator uint32_t() const {
        return m_uColor;
    }
    inline operator std::string() const {
        char buf[64];
        sprintf(buf, "clr{%i,%i,%i,%i}", r(),g(),b(),a());
        return std::string(buf);
    }
    uint8_t operator [](int idx) const {
        switch (idx)
        {
        case 0:
            return r();
        case 1 :
            return g();
        case 2:
            return b();
        case 4: return a();
        default:
            return 0xffu;
        }
    }
    uint8_t& operator [](int idx){
        switch (idx)
        {
        case 0:
            return *(uint8_t*)(&m_uColor + 0x3);
        case 1 :
            return *(uint8_t*)(&m_uColor + 0x2);
        case 2:
            return *(uint8_t*)(&m_uColor + 0x1);
        case 4: return *(uint8_t*)(&m_uColor + 0x0);
        default:
            return*(uint8_t*)(&m_uColor + 0x0);
        }
    }
    inline Color operator=(const SDL_Color& rhs) const {
        return Color(rhs);
    }
    inline Color operator=(const uint32_t& rhs) const {
        return Color(rhs);
    }


    inline bool operator==(const Color& rhs) const {
        return m_uColor == rhs.m_uColor;
    }
    inline bool operator==(const uint32_t& rhs) const {
        return m_uColor == rhs;
    }
    inline Color operator+(const Color& rhs) const {
        const uint32_t alpha = a();  
        const uint32_t oalpha = 255 - alpha;
        uint32_t ret = ((alpha * r() + rhs.r() * oalpha) / 255) << 24 |
                    ((alpha * g() + rhs.g() * oalpha) / 255) << 16 |
                    ((alpha * b() + rhs.b() * oalpha) / 255) << 8 |
                    ((alpha * a() + rhs.a() * oalpha) / 255);
        return Color(ret);
    }

    inline Color operator*(const float v) const {
        return Color( r() * v, g() * v, b() * v, a());
    }
    inline Color operator*=(const float v) const {
        return Color( r() * v, g() * v, b() * v, a());
    }
    inline Color operator/(const float v) const {
        return Color( r() / v, g() / v, b() / v, a());
    }
    inline Color operator%(const float v) const { //divides with alpha 
        return Color( r() / v, g() / v, b() / v, a() / v);
    }
    inline Color operator/=(const float v) const {
        return Color( r() / v, g() / v, b() / v, a());
    }

    static constexpr Color None() { return Color(0,0,0,0); }
    //Opaque
    static constexpr Color White() { return Color(255,255,255,255); }
    static constexpr Color Black() { return Color(0,0,0,255); }
    static constexpr Color Cyan() { return Color(0,255,255,255); }
    static constexpr Color Red() { return Color(255,0,0,255); }
    static constexpr Color Green() { return Color(0,255,0,255); }
    static constexpr Color Blue() { return Color(0,0,255,255); }
    static constexpr Color Grey() { return Color(127,127,127,255); }
    static constexpr Color Yellow() { return Color(255, 255, 0, 255); }
    static constexpr Color Magenta() { return Color(255, 0, 255, 255); }
    static constexpr Color Orange() { return Color(255, 165, 0, 255); }
    static constexpr Color Purple() { return Color(128, 0, 128, 255); }
    static constexpr Color Brown() { return Color(165, 42, 42, 255); }
    static constexpr Color Pink() { return Color(255, 192, 203, 255); }
    static constexpr Color Lime() { return Color(0, 255, 0, 255); }
    static constexpr Color SkyBlue() { return Color(135, 206, 235, 255); }
    static constexpr Color Beige() { return Color(245, 245, 220, 255); }
    static constexpr Color Mint() { return Color(189, 252, 201, 255); }
    static constexpr Color Lavender() { return Color(230, 230, 250, 255); }
    static constexpr Color Coral() { return Color(255, 127, 80, 255); }
    static constexpr Color Navy() { return Color(0, 0, 128, 255); }
    static constexpr Color Teal() { return Color(0, 128, 128, 255); }
    static constexpr Color Olive() { return Color(128, 128, 0, 255); }
    static constexpr Color Maroon() { return Color(128, 0, 0, 255); }
    static constexpr Color Charcoal() { return Color(54, 69, 79, 255); }
    static constexpr Color Indigo() { return Color(75, 0, 130, 255); }
    static constexpr Color Ivory() { return Color(255, 255, 240, 255); }
    static constexpr Color Silver() { return Color(192, 192, 192, 255); }

    //Alpha Using For lighting, thanks chatgpt
    static constexpr Color SoftWhiteLight() { return Color(255, 255, 224, 180); } // Soft white with semi-opacity
    static constexpr Color WarmYellowLight() { return Color(255, 239, 213, 180); } // Warm yellow light
    static constexpr Color CoolBlueLight() { return Color(173, 216, 230, 180); } // Cool blue, for a moonlight effect
    static constexpr Color SoftGreenLight() { return Color(144, 238, 144, 180); } // Soft green, for a natural, eco-friendly light
    static constexpr Color DimRedLight() { return Color(255, 99, 71, 180); } // Dim red, for a subtle, warm effect
    static constexpr Color PalePurpleLight() { return Color(216, 191, 216, 180); } // Pale purple, for a mystical ambience
    static constexpr Color SunsetOrangeLight() { return Color(255, 165, 0, 180); } // Sunset orange, for a dusk-like feel
    static constexpr Color Moonlight() { return Color(210, 210, 255, 180); } // Moonlight color, for a night scene
    static constexpr Color CandleLight() { return Color(255, 147, 41, 180); } // Candlelight, for a cozy and warm atmosphere
    static constexpr Color FluorescentLight() { return Color(204, 255, 255, 180); } // Fluorescent light, for a stark, bright effect

    
private:
    uint32_t m_uColor;
};

