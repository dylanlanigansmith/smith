#pragma once
#include <cstdint>
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <stdarg.h>
#include <memory>
#include <cxxabi.h>
#include <typeinfo>
#include <algorithm>
#include <functional>
#include <cassert>


#define M_PI		3.14159265358979323846f
#define M_RADPI		57.295779513082f
#define M_PI_F		((float)(M_PI))	
#define RAD2DEG( x )  ( (float)(x) * (float)(180.f / M_PI_F) )
#define DEG2RAD( x )  ( (float)(x) * (float)(M_PI_F / 180.f) )
#define RAD2DEGNUM 57.295779513082f

#define quote(x) #x