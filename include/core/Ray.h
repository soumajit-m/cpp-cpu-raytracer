#pragma once

#include "math/Vec3.h"

struct Ray
{
    Vec3 origin;
    Vec3 direction;
    Vec3 invDirection;

    Ray(const Vec3& origin, const Vec3& direction);

    Vec3 at(float distance) const;
};
