#include "scene/Triangle.h"

Triangle::Triangle(
    const Vec3& v0, const Vec3& v1, const Vec3& v2,
    const Vec3& n0, const Vec3& n1, const Vec3& n2,
    const Material& mat
)
    : v0(v0), v1(v1), v2(v2), n0(n0), n1(n1), n2(n2), material(mat)
{
}
