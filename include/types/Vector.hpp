#pragma once
#include <common.hpp>
#include <cstdio>

enum Cardinal_Directions : int
{
  NORTH = 0, // -y
  EAST = 1,  //+x
  SOUTH = 2, //+y
  WEST = 3   //-x
};
class Vector2
{
public:
  Vector2() { x = y = 0.0; }
  Vector2(double x, double y) : x(x), y(y) {}

  double x, y;
  Vector2 operator-(const Vector2 &rhs) const
  {
    return {x - rhs.x, y - rhs.y};
  }
  Vector2 operator+(const Vector2 &rhs) const
  {
    return {x + rhs.x, y + rhs.y};
  }
  Vector2 operator*(const double &rhs) const
  {
    return {x * rhs, y * rhs};
  }
  Vector2 operator/(const double &rhs) const
  {
    return {x / rhs, y / rhs};
  }

  bool operator==(const Vector2 &rhs) const
  {
    return (x == rhs.x) && (y == rhs.y);
  }
  bool operator!=(const Vector2 &rhs) const
  {
    return (x != rhs.x) && (y != rhs.y);
  }
  bool operator<(const Vector2 &rhs) const
  {
    return (x < rhs.x) || (x == rhs.x && y < rhs.y);
  }
  bool operator>(const Vector2 &rhs) const
  {
    return (x > rhs.x) || (x == rhs.x && y > rhs.y);
  }
  double &operator[](uint8_t i)
  {
    switch (i)
    {
    case 0:
      return x;
    case 1:
      return y;
    default:
      return x;
    }
  }
  double operator[](uint8_t i) const
  {
    switch (i)
    {
    case 0:
      return x;
    case 1:
      return y;
    default:
      return x;
    }
  }
  inline double Length() const noexcept { return std::sqrt(x * x + y * y); }
  inline double LengthSqr() const noexcept { return (x * x + y * y); }
  inline Vector2 Normalize() const
  {
    double mag = Length();
    return Vector2(x / mag, y / mag);
  }
  static double dot(const Vector2 &a, const Vector2 &b)
  {
    return a.x * b.x + a.y * b.y;
  }
  double dot(const Vector2 &b) const
  {
    return x * b.x + y * b.y;
  }
  double dotClamped(const Vector2 &b) const
  {
    return std::clamp(x * b.x + y * b.y, -1.0, 1.0);
  }
  double cross(const Vector2 &other) const
  {
    return x * other.y - y * other.x;
  }
  const char *s() const
  {
    static std::ostringstream oss;
    oss.clear();
    oss << "{" << x << ", " << y << "}";

    return oss.str().c_str();
  }
  void rotate(double angleDegrees)
  {
    double radians = angleDegrees * M_PI / 180.0; // Convert to radians
    double cosTheta = cos(radians);
    double sinTheta = sin(radians);

    double newX = x * cosTheta - y * sinTheta;
    double newY = x * sinTheta + y * cosTheta;

    x = newX;
    y = newY;
  }
  Vector2 lerp(const Vector2 &target, double factor) const
  {
    return Vector2(x + factor * (target.x - x), y + factor * (target.y - y));
  }

  static inline Vector2 intersect_segs(Vector2 a0, Vector2 a1, Vector2 b0, Vector2 b1)
  { /// yikes
    const double d =
        ((a0.x - a1.x) * (b0.y - b1.y)) - ((a0.y - a1.y) * (b0.x - b1.x));

    if (fabsf(d) < 0.000001f)
    {
      return (Vector2){NAN, NAN};
    }

    const double
        t = (((a0.x - b0.x) * (b0.y - b1.y)) - ((a0.y - b0.y) * (b0.x - b1.x))) / d,
        u = (((a0.x - b0.x) * (a0.y - a1.y)) - ((a0.y - b0.y) * (a0.x - a1.x))) / d;
    return (t >= 0 && t <= 1 && u >= 0 && u <= 1) ? ((Vector2){
                                                        a0.x + (t * (a1.x - a0.x)),
                                                        a0.y + (t * (a1.y - a0.y))})
                                                  : ((Vector2){NAN, NAN});
  }

  static inline bool closeEnough(const Vector2& cur, const Vector2& goal, double tol = 0.35){
      if( (goal - cur).LengthSqr() <= tol*tol ){
          return true;
      }
      return false;
  }
};
template <>
struct std::hash<Vector2>
{
  std::size_t operator()(const Vector2 &k) const noexcept
  {
    using std::hash;
    using std::size_t;
    using std::string;

    return ((hash<float>()((k.y + 1) * (k.x + 1)) ^ (hash<float>()(k.y) << 1)) >> 1) ^ (hash<float>()(k.x) << 1);
  }
};
struct BBoxAABB
{
  Vector2 min;
  Vector2 max;
};

struct Ray_t
{
  Vector2 origin;
  Vector2 direction; // normalized
};

struct Line_t
{
  Vector2 p0;
  Vector2 p1;

  Vector2 &start() { return p0; }
  Vector2 &end() { return p1; }
  Vector2 start() const { return p0; }
  Vector2 end() const { return p1; }
};

class IVector2
{
public:
  IVector2() { x = y = 0; }
  IVector2(int x, int y) : x(x), y(y) {}
  IVector2(double x, double y) : x(int(x)), y(int(y)) {}
  IVector2(const Vector2 in) : x((int)in.x), y((int)in.y) {} // this is not fair rounding
  int x, y;

  int h() { return y; }
  int w() { return x; }
  IVector2 operator-(const IVector2 &rhs) const
  {
    return {x - rhs.x, y - rhs.y};
  }
  IVector2 operator+(const IVector2 &rhs) const
  {
    return {x + rhs.x, y + rhs.y};
  }
  bool operator==(const IVector2 &rhs) const
  {
    return (x == rhs.x) && (y == rhs.y);
  }
  bool operator!=(const IVector2 &rhs) const
  {
    return (x != rhs.x) || (y != rhs.y);
  }
  bool operator<(const IVector2 &rhs) const
  {
    return (x < rhs.x) || (x == rhs.x && y < rhs.y);
  }
  bool operator>(const IVector2 &rhs) const
  {
    return (x > rhs.x) || (x == rhs.x && y > rhs.y);
  }

  std::string str() const
  {
    std::ostringstream oss;
    oss << "{" << x << ", " << y << "}";

    return oss.str();
  }
  const char *s() const
  {
    static std::ostringstream oss;
    oss.clear();
    oss << "{" << x << ", " << y << "}";

    return oss.str().c_str();
  }
  static IVector2 Rounded(double x, double y) { return Rounded({x, y}); }
  static IVector2 Rounded(const Vector2 &v)
  {
    return IVector2(static_cast<int>(std::round(v.x)), static_cast<int>(std::round(v.y)));
  }

  operator Vector2(){
    return Vector2(x,y);
  }
};
template <>
struct std::hash<IVector2>
{
  std::size_t operator()(const IVector2 &k) const noexcept
  {
    using std::hash;
    using std::size_t;
    using std::string;

    // Compute individual hash values for first,
    // second and third and combine them using XOR
    // and bit shifting:

    return ((hash<int>()((k.y + 1) * (k.x + 1)) ^ (hash<int>()(k.y) << 1)) >> 1) ^ (hash<int>()(k.x) << 1);
  }
};

class Vector
{
public:
 double x, y, z;
  Vector(float v) : x(v), y(v), z(v) {}
  Vector(const double xx, const double yy, const double zz) : x(xx), y(yy), z(zz) {}
  Vector(const Vector2 &dumb)
  {
    x = dumb.x;
    y = dumb.y;
    z = 0.f;
  }
  Vector(const Vector &dumb)
  {
    x = dumb.x;
    y = dumb.y;
    z = dumb.z;
  }
  Vector()
  {
    x = 0.f;
    y = 0.f;
    z = 0.f;
  }

  bool IsValid() const
  {
    return (std::isfinite(this->x) && std::isfinite(this->y) && std::isfinite(this->z));
  }

  inline double Length2D() const noexcept { return std::sqrt(x * x + y * y); }
  inline double Length3D() const noexcept { return std::sqrt(x * x + y * y + z * z); }
  inline double LengthSqr() const noexcept { return x * x + y * y + z * z; }
  inline void Min(const Vector &rhs)
  {
    x = std::min(x, rhs.x);
    y = std::min(y, rhs.y);
    z = std::min(z, rhs.z);
  }
  inline void Max(const Vector &rhs)
  {
    x = std::max(x, rhs.x);
    y = std::max(y, rhs.y);
    z = std::max(z, rhs.z);
  }
  inline Vector Normalize() const
  {
    double mag = Length3D();
    return Vector(x / mag, y / mag, z / mag);
  }
  inline double Dot(const Vector &other) const
  {
    return x * other.x + y * other.y + z * other.z;
  }
  inline double AbsDot(const Vector &rhs) const
  {
    return std::abs(x * rhs.x) + std::abs(y * rhs.y) + std::abs(z * rhs.z);
  }

  bool operator==(Vector const &a) const
  {
    return x == a.x && y == a.y && z == a.z;
  }
  Vector operator+(Vector const &a) const
  {
    return Vector(x + a.x, y + a.y, z + a.z);
  }
  Vector operator+(double const &a) const
  {
    return Vector(x + a, y + a, z + a);
  }
  Vector operator+=(Vector const &a) const
  {
    return Vector(x + a.x, y + a.y, z + a.z);
  }
  Vector operator-(Vector const &a) const
  {
    return Vector(x - a.x, y - a.y, z - a.z);
  }
  Vector operator*(double s) const
  {
    return Vector(x * s, y * s, z * s);
  }

 double &operator[](uint8_t i)
  {
    switch (i)
    {
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    default:
      return z;
    }
  }
  double operator[](uint8_t i) const
  {
    switch (i)
    {
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    default:
      return z;
    }
  }

  operator Vector2() const
  {
    return {x, y};
  }
};