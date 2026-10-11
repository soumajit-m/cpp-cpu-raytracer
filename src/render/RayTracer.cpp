#include "render/RayTracer.h"
#include <limits>

RayTraceResult RayTracer::trace(const Ray& ray, const BVH& bvh, bool smoothShading) const
{
    RayTraceResult bestResult;
    bestResult.hit = false;
    bestResult.distance = std::numeric_limits<float>::max();

    if (bvh.getNodes().empty()) return bestResult;

    intersectBVH(ray, 0, bvh, bestResult, smoothShading);

    return bestResult;
}

void RayTracer::intersectBVH(const Ray& ray, int nodeIdx, const BVH& bvh, RayTraceResult& bestResult, bool smoothShading) const
{
    const BVHNode& node = bvh.getNodes()[nodeIdx];
    float tHit;
    
    if (!node.bounds.intersect(ray, tHit)) return;
    if (tHit >= bestResult.distance) return;

    if (node.isLeaf())
    {
        constexpr float EPSILON = 0.000001f;

        for (int i = 0; i < node.triCount; ++i)
        {
            const Triangle& tri = bvh.getTriangles()[node.leftFirst + i];

            Vec3 edge1 = tri.v1 - tri.v0;
            Vec3 edge2 = tri.v2 - tri.v0;
            Vec3 h = ray.direction.cross(edge2);
            float a = edge1.dot(h);

            if (a > -EPSILON && a < EPSILON) continue;

            float f = 1.0f / a;
            Vec3 s = ray.origin - tri.v0;
            float u = f * s.dot(h);
            if (u < 0.0f || u > 1.0f) continue;

            Vec3 q = s.cross(edge1);
            float v = f * ray.direction.dot(q);
            if (v < 0.0f || u + v > 1.0f) continue;

            float t = f * edge2.dot(q);
            
            if (t > EPSILON && t < bestResult.distance)
            {
                bestResult.hit = true;
                bestResult.distance = t;
                bestResult.material = tri.material;

                Vec3 normal;
                if (smoothShading)
                {
                    float w = 1.0f - u - v;
                    normal = (tri.n0 * w + tri.n1 * u + tri.n2 * v).normalized();
                }
                else
                {
                    normal = edge1.cross(edge2).normalized();
                }

                if (normal.dot(ray.direction) > 0.0f)
                {
                    normal = normal * -1.0f;
                }
                bestResult.normal = normal;
            }
        }
    }
    else
    {
        intersectBVH(ray, node.leftFirst, bvh, bestResult, smoothShading);
        intersectBVH(ray, node.leftFirst + 1, bvh, bestResult, smoothShading);
    }
}

bool RayTracer::traceShadow(const Ray& ray, const BVH& bvh, float maxDistance) const
{
    if (bvh.getNodes().empty()) return false;
    return intersectBVHShadow(ray, 0, bvh, maxDistance);
}

bool RayTracer::intersectBVHShadow(const Ray& ray, int nodeIdx, const BVH& bvh, float maxDistance) const
{
    const BVHNode& node = bvh.getNodes()[nodeIdx];
    float tHit;

    // If the ray misses the bounding box, or the box is further than the light, skip it
    if (!node.bounds.intersect(ray, tHit) || tHit >= maxDistance) return false;

    if (node.isLeaf())
    {
        constexpr float EPSILON = 0.000001f;

        for (int i = 0; i < node.triCount; ++i)
        {
            const Triangle& tri = bvh.getTriangles()[node.leftFirst + i];

            Vec3 edge1 = tri.v1 - tri.v0;
            Vec3 edge2 = tri.v2 - tri.v0;
            Vec3 h = ray.direction.cross(edge2);
            float a = edge1.dot(h);

            if (a > -EPSILON && a < EPSILON) continue;

            float f = 1.0f / a;
            Vec3 s = ray.origin - tri.v0;
            float u = f * s.dot(h);
            if (u < 0.0f || u > 1.0f) continue;

            Vec3 q = s.cross(edge1);
            float v = f * ray.direction.dot(q);
            if (v < 0.0f || u + v > 1.0f) continue;

            float t = f * edge2.dot(q);

            // --- THE ANY-HIT MAGIC ---
            // We found a valid hit! Instantly abort and return true.
            // No normals, no materials, no checking if there's a closer triangle.
            if (t > EPSILON && t < maxDistance) return true;
        }
        return false;
    }

    // If the left child returns true, instantly abort and skip the right child
    if (intersectBVHShadow(ray, node.leftFirst, bvh, maxDistance)) return true;
    return intersectBVHShadow(ray, node.leftFirst + 1, bvh, maxDistance);
}