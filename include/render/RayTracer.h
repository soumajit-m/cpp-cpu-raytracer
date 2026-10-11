#pragma once
#include "core/Ray.h"
#include "scene/BVH.h"
#include "scene/Material.h"

struct RayTraceResult
{
    bool hit;
    float distance;
    Vec3 normal;
    Material material;
};

class RayTracer
{
public:
    RayTraceResult trace(const Ray& ray, const BVH& bvh, bool smoothShading) const;

    // NEW: A highly optimized, boolean-only trace for shadows
    bool traceShadow(const Ray& ray, const BVH& bvh, float maxDistance) const;

private:
    void intersectBVH(const Ray& ray, int nodeIdx, const BVH& bvh, RayTraceResult& bestResult, bool smoothShading) const;

    // NEW: The early-exit traversal
    bool intersectBVHShadow(const Ray& ray, int nodeIdx, const BVH& bvh, float maxDistance) const;
};