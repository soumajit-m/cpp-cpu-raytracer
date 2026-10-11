#include "scene/BVH.h"
#include <algorithm>

BVH::BVH(const std::vector<Triangle>& tris)
{
    triangles = tris;
    
    if (triangles.empty()) return;

    // A tree with N leaves has exactly 2N-1 total nodes maximum
    nodes.resize(triangles.size() * 2 - 1);
    
    BVHNode& root = nodes[0];
    root.leftFirst = 0;
    root.triCount = static_cast<int>(triangles.size());
    nodesUsed = 1;

    updateNodeBounds(0);
    subdivide(0);
}

void BVH::updateNodeBounds(int nodeIdx)
{
    BVHNode& node = nodes[nodeIdx];
    node.bounds = AABB();
    
    for (int i = 0; i < node.triCount; i++)
    {
        const Triangle& tri = triangles[node.leftFirst + i];
        node.bounds.grow(tri.v0);
        node.bounds.grow(tri.v1);
        node.bounds.grow(tri.v2);
    }
}

void BVH::subdivide(int nodeIdx)
{
    BVHNode& node = nodes[nodeIdx];
    
    // Stop subdividing if the box only has a couple of triangles
    if (node.triCount <= 2) return;

    // Find the longest axis of the bounding box
    Vec3 extent = node.bounds.max - node.bounds.min;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > (axis == 0 ? extent.x : extent.y)) axis = 2;

    float splitPos = (axis == 0 ? node.bounds.min.x : (axis == 1 ? node.bounds.min.y : node.bounds.min.z)) 
                     + (axis == 0 ? extent.x : (axis == 1 ? extent.y : extent.z)) * 0.5f;

    // Partition the triangles based on which side of the split plane their centroid falls
    int i = node.leftFirst;
    int j = i + node.triCount - 1;
    
    while (i <= j)
    {
        Vec3 centroid = (triangles[i].v0 + triangles[i].v1 + triangles[i].v2) * 0.333333f;
        float centroidAxisVal = (axis == 0 ? centroid.x : (axis == 1 ? centroid.y : centroid.z));
        
        if (centroidAxisVal < splitPos)
        {
            i++;
        }
        else
        {
            std::swap(triangles[i], triangles[j--]);
        }
    }

    int leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.triCount) return; // Cannot subdivide further

    // Create child nodes
    int leftChildIdx = nodesUsed++;
    int rightChildIdx = nodesUsed++;

    nodes[leftChildIdx].leftFirst = node.leftFirst;
    nodes[leftChildIdx].triCount = leftCount;
    
    nodes[rightChildIdx].leftFirst = i;
    nodes[rightChildIdx].triCount = node.triCount - leftCount;

    // Turn current node into an internal node
    node.leftFirst = leftChildIdx;
    node.triCount = 0;

    updateNodeBounds(leftChildIdx);
    updateNodeBounds(rightChildIdx);

    subdivide(leftChildIdx);
    subdivide(rightChildIdx);
}
