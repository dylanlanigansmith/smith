#pragma once
#include <common.hpp>
#include <nlohmann/json.hpp>
#include <types/Vector.hpp>

namespace Util
{
    using json =  nlohmann::json; 
    inline nlohmann::json ivec2json(const IVector2& vec){
        auto ret = json::array();
        ret = { vec.x, vec.y};
        return ret; 
    }

    inline nlohmann::json vec3json(const Vector& vec){
        auto ret = json::array();
        ret = { vec.x, vec.y, vec.z};
        return ret; 
    }

    inline nlohmann::json vec2json(const Vector2& vec){
        auto ret = json::array();
        ret = { vec.x, vec.y};
        return ret; 
    }
    inline nlohmann::json bboxjson(const BBoxAABB& bb){
        auto ret = json::array();
        ret = { vec2json(bb.min), vec2json(bb.max)};
        return ret; 
    }

    inline nlohmann::json double3json(double a, double b, double c){
        auto ret = json::array();
        ret = { a, b, c};
        return ret; 
    }
    inline nlohmann::json double2int1json(double a, double b, int c){
        auto ret = json::array();
        ret = { a, b, c};
        return ret; 
    }
    inline nlohmann::json string_array_json(const std::vector<std::string>& vs){
        auto ret = json::array();
        for(const auto& str : vs){
            ret.push_back(str);
        }
        return ret; 
    }

}