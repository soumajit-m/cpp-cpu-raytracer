#pragma once

#include "math/Vec3.h"
#include "core/Ray.h"

struct AABB
{
    Vec3 min;
    Vec3 max;

    AABB();
    void grow(const Vec3& p);
    bool intersect(const Ray& ray, float& tMinHit) const;
};
