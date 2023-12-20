#pragma once
#include <common.hpp>

class hEntity
{
public:
    hEntity(uint32_t value = 0) : value(value) {}
    operator uint32_t() const { return value; }
    operator std::string() const { return std::to_string(value); }

    bool operator==(const hEntity& rhs) const {
        return value == rhs.value;
    }
    bool operator==(const uint32_t& rhs) const {
        return value == rhs;
    }

    hEntity operator=(const uint32_t& rhs) const {
        return hEntity(rhs);
    }
    //no proper assignment because this shouldnt be changed!
private:
    uint32_t value;
};
template <>
struct std::hash<hEntity>
{
  std::size_t operator()(const hEntity k) const noexcept{
    using std::hash;
    std::size_t j = static_cast<std::size_t>(k);
    return j << 32;
  }
};

#define ENT_INVALID 0xffffffffU 