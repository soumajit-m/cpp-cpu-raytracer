#pragma once

#include <vector>
#include "math/AABB.h"
#include "scene/Triangle.h"

struct BVHNode
{
    AABB bounds;
    int leftFirst; // Left child node index, OR first triangle index if it is a leaf
    int triCount;  // 0 if internal node, > 0 if leaf node
    
    bool isLeaf() const { return triCount > 0; }
};

class BVH
{
public:
    BVH(const std::vector<Triangle>& tris);
    
    const std::vector<BVHNode>& getNodes() const { return nodes; }
    const std::vector<Triangle>& getTriangles() const { return triangles; }

private:
    std::vector<Triangle> triangles;
    std::vector<BVHNode> nodes;
    int nodesUsed;

    void updateNodeBounds(int nodeIdx);
    void subdivide(int nodeIdx);
};
