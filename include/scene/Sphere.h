#pragma once
#include <vector>
#include "math/Vec3.h"
#include "scene/Triangle.h"
#include "scene/Material.h"

class Sphere
{
public:
    static std::vector<Triangle> generateMesh(const Vec3& center, float radius, int rings, int segments, const Material& material = Material());
};
