#pragma once

class Vector2 
{
public:
    Vector2(){ x = y = 0.0 ;}
    Vector2( double x, double y) : x(x), y(y) {}

    double x,y;

};

class IVector2 
{
public:
    IVector2(){ x = y = 0 ;}
    IVector2( int x, int y) : x(x), y(y) {}
    IVector2( double x, double y) : x(int(x)), y(int(y)) {}
    IVector2(const Vector2 in) : x((int)in.x), y((int)in.y) {}
    int x,y;

    int h() { return y; }
    int w() { return x ;}

};

class Vector{ 
  public:
    float x,y,z;
    Vector(float v) : x(v), y(v), z(v) {}
    Vector(const float xx, const float yy, const float zz) : x(xx), y(yy), z(zz){}
    Vector(const Vector2& dumb) { x = dumb.x; y =dumb.y; z = 0.f;}
    Vector(const Vector& dumb) { x = dumb.x; y =dumb.y; z = dumb.z;}
    Vector() { x = 0.f; y = 0.f; z = 0.f;}
  
  bool IsValid() const
	{
		return (std::isfinite(this->x) && std::isfinite(this->y) && std::isfinite(this->z));
	}



   
    inline float Length2D() const noexcept { return std::sqrt(x * x + y * y); }
    inline float Length3D() const noexcept { return std::sqrt(x * x + y * y + z * z); }
    inline float LengthSqr() const noexcept { return x * x + y * y + z * z; }
    inline void Min(const Vector& rhs) { x = std::min(x, rhs.x); y =  std::min(y, rhs.y); z = std::min(z, rhs.z); }
    inline void Max(const Vector& rhs) { x = std::max(x, rhs.x); y=  std::max(y, rhs.y); z = std::max(z, rhs.z); }
    inline Vector Normalize() const {
      float mag = Length3D();
      return Vector(x / mag, y / mag, z / mag);
    }
     inline float Dot(const Vector& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    inline float AbsDot(const Vector& rhs) const {
      return std::abs(x * rhs.x) + std::abs(y * rhs.y) + std::abs(z * rhs.z); 
    }
    


    
    Vector operator+(Vector const& a ) const{
      return Vector(x + a.x, y + a.y, z + a.z);
    }
    Vector operator+(double const& a ) const{
      return Vector(x + a, y + a, z + a);
    }
    Vector operator-(Vector const& a ) const{
      return Vector(x - a.x, y - a.y, z - a.z);
    }
    Vector operator*(double s) const{
      return Vector(x * s, y * s, z * s);
    }
    
    float &operator[](uint8_t i){
      switch(i){
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default:
          return z;
      }
    }
    float operator[](uint8_t i) const {
      switch(i){
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default:
          return z;
      }
    }
    
};