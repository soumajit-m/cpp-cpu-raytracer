#include "scene/Sphere.h"
#include <cmath>

std::vector<Triangle> Sphere::generateMesh(const Vec3& center, float radius, int rings, int segments, const Material& material)
{
    std::vector<Triangle> triangles;
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    
    constexpr float PI = 3.14159265358979323846f;

    for (int i = 0; i <= rings; ++i)
    {
        float phi = PI * static_cast<float>(i) / static_cast<float>(rings);
        for (int j = 0; j <= segments; ++j)
        {
            float theta = 2.0f * PI * static_cast<float>(j) / static_cast<float>(segments);
            Vec3 pos(
                center.x + radius * std::sin(phi) * std::cos(theta),
                center.y + radius * std::cos(phi),
                center.z + radius * std::sin(phi) * std::sin(theta)
            );
            vertices.push_back(pos);
            normals.push_back((pos - center).normalized());
        }
    }

    for (int i = 0; i < rings; ++i)
    {
        for (int j = 0; j < segments; ++j)
        {
            int current = i * (segments + 1) + j;
            int next = current + 1;
            int bottom = (i + 1) * (segments + 1) + j;
            int bottomNext = bottom + 1;

            triangles.push_back(Triangle(
                vertices[current], vertices[bottom], vertices[bottomNext],
                normals[current], normals[bottom], normals[bottomNext],
                material
            ));
            triangles.push_back(Triangle(
                vertices[current], vertices[bottomNext], vertices[next],
                normals[current], normals[bottomNext], normals[next],
                material
            ));
        }
    }
    return triangles;
}
