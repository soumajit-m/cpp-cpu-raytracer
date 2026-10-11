#include "core/Ray.h"

Vec3 Ray::at(float distance) const
{
    return origin + direction * distance;
}

Ray::Ray(const Vec3& origin, const Vec3& direction)
    : origin(origin), direction(direction)
{
    // Precompute division to save millions of CPU cycles
    invDirection.x = 1.0f / direction.x;
    invDirection.y = 1.0f / direction.y;
    invDirection.z = 1.0f / direction.z;
}