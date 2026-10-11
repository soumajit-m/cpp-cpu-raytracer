#pragma once

#include <vector>

#include "core/Ray.h"
#include "scene/Sphere.h"

struct RayMarchResult
{
    bool hit;
    float distance;
    int steps;
    int objectIndex;

    // Smallest distance from the ray to any
    // object surface during the march.
    float closestSurfaceDistance;

    // Distance travelled along the ray when
    // closestSurfaceDistance was found.
    float closestApproachDistance;
};

class RayMarcher
{
public:

    RayMarchResult march(
        const Ray& ray,
        const std::vector<Sphere>& spheres
    ) const;

private:

    static constexpr int MAX_STEPS = 128;

    static constexpr float MAX_DISTANCE = 100.0f;

    // Actual surface hit threshold.
    static constexpr float SURFACE_DISTANCE = 0.001f;
};