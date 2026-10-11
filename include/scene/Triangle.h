#pragma once
#include "math/Vec3.h"
#include "scene/Material.h"

struct Triangle
{
    Vec3 v0; Vec3 v1; Vec3 v2;
    Vec3 n0; Vec3 n1; Vec3 n2;
    Material material;

    Triangle(
        const Vec3& v0, const Vec3& v1, const Vec3& v2,
        const Vec3& n0, const Vec3& n1, const Vec3& n2,
        const Material& mat = Material()
    );
};
