#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include <cstring>
#include <unordered_map>
#include <map>
#include <vector>
#include <cmath>
#include <stdarg.h>
#include <memory>
#include <cxxabi.h>
#include <typeinfo>
#include <algorithm>
#include <functional>
#include <cassert>
#include <limits.h>
#include <float.h>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>




#define M_RADPI		57.295779513082f
#define M_PI_F		((float)(M_PI))	
#define RAD2DEG( x )  ( (float)(x) * (float)(180.f / M_PI_F) )
#define DEG2RAD( x )  ( (float)(x) * (float)(M_PI_F / 180.f) )
#define RAD2DEGNUM 57.295779513082f

#define quote(x) #x


