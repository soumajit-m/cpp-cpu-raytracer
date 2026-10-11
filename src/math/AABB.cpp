#include "math/AABB.h"
#include <algorithm>
#include <limits>

AABB::AABB()
{
    min = Vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    max = Vec3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
}

void AABB::grow(const Vec3& p)
{
    min.x = std::min(min.x, p.x);
    min.y = std::min(min.y, p.y);
    min.z = std::min(min.z, p.z);
    
    max.x = std::max(max.x, p.x);
    max.y = std::max(max.y, p.y);
    max.z = std::max(max.z, p.z);
}

bool AABB::intersect(const Ray& ray, float& tMinHit) const
{
    float t1 = (min.x - ray.origin.x) * ray.invDirection.x;
    float t2 = (max.x - ray.origin.x) * ray.invDirection.x;
    float tmin = std::min(t1, t2);
    float tmax = std::max(t1, t2);

    t1 = (min.y - ray.origin.y) * ray.invDirection.y;
    t2 = (max.y - ray.origin.y) * ray.invDirection.y;
    tmin = std::max(tmin, std::min(t1, t2));
    tmax = std::min(tmax, std::max(t1, t2));

    t1 = (min.z - ray.origin.z) * ray.invDirection.z;
    t2 = (max.z - ray.origin.z) * ray.invDirection.z;
    tmin = std::max(tmin, std::min(t1, t2));
    tmax = std::min(tmax, std::max(t1, t2));

    if (tmax >= tmin && tmax >= 0.0f)
    {
        tMinHit = tmin > 0.0f ? tmin : 0.0f;
        return true;
    }
    return false;
}
