#pragma once
#include <common.hpp>
#include <stdio.h>
#include <types/Vector.hpp>

namespace Util
{
    static const std::string stringf(const char *fmt, ...)
    { 
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        return std::string(buf);
    }
    static inline std::string stripPath(const std::string& path){
         return path.substr(path.find_last_of("/") + 1);
    }

    static inline int SemiRandRange(int min, int max){
        return min + ( std::rand() % ( max - min + 1 ) );
    }

    static bool RayIntersectsBox(const Ray_t& ray, const BBoxAABB& box) {
        float tMin = 0.0f; // start of the ray
        float tMax = FLT_MAX; // end of the ray

        // For X and Y axes
        for (int i = 0; i < 2; i++) {
            float invD = 1.0f / ray.direction[i];
            float t0 = (box.min[i] - ray.origin[i]) * invD;
            float t1 = (box.max[i] - ray.origin[i]) * invD;

            if (invD < 0.0f) {
                std::swap(t0, t1);
            }

            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;

            if (tMin > tMax) {
                return false;
            }
        }
        return true;
    }

    static bool RayIntersectsCircle(const Ray_t& ray, const Vector2& center, double radius)
    {
        Vector2 circleToRayOrigin = center - ray.origin;
        double t = Vector2::dot(circleToRayOrigin, ray.direction); // Project onto ray
        Vector2 closestPoint = ray.origin +  ray.direction * t;

        // Check if this point is within the circle's radius
        double distanceSquared = (center - closestPoint).LengthSqr();
        return distanceSquared <= (radius * radius);
    }
    static bool isPointInBBox(const Vector2& P, const BBoxAABB& box) {
        return (P.x >= box.min.x && P.x <= box.max.x) &&
            (P.y >= box.min.y && P.y <= box.max.y);
    }

    static IVector2 getPixelOffsetInsideAABB(const IVector2& P, const BBoxAABB& box) {
            IVector2 offset;
            offset.x = P.x - box.min.x;
            offset.y = P.y - box.min.y;
            return offset;
        }
    static IVector2  getPixelAbsFromAABBOffset(const IVector2& RelativePos, const BBoxAABB& box) {
        IVector2 absolutePosition;
        absolutePosition.x = RelativePos.x + box.min.x;
        absolutePosition.y = RelativePos.y + box.min.y;
        return absolutePosition;
    }

    static bool RayIntersectsLineSegment(const Ray_t& ray, const Line_t& wall, Vector2& intersection) {
        // Define line segment as AB
        Vector2 A = wall.start();
        Vector2 B = wall.end();

            // Ray direction
        Vector2 rayDir = ray.direction;

        // Ray-Line Segment intersection formula
        Vector2 AtoB = { B.x - A.x, B.y - A.y }; // Direction of the line segment
        Vector2 AtoOrigin = { ray.origin.x - A.x, ray.origin.y - A.y };

        // Calculate the determinant (2D cross product)
        float det = rayDir.x * AtoB.y - rayDir.y * AtoB.x;

        // If the determinant is zero, ray is parallel to the line segment
        if (det == 0) {
            return false;
        }

        // Solve for parameters t (for ray) and u (for line)
        float t = (AtoB.x * AtoOrigin.y - AtoB.y * AtoOrigin.x) / det;
        float u = (rayDir.x * AtoOrigin.y - rayDir.y * AtoOrigin.x) / det;

        // Check if 0 <= u <= 1 and t >= 0
        if (u >= 0 && u <= 1 && t >= 0) {
            intersection = { ray.origin.x + t * rayDir.x, ray.origin.y + t * rayDir.y };
            return true;
        }

        return false;
    }
}