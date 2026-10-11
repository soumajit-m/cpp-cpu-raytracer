#pragma once
#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#include "core/Camera.h"
#include "scene/BVH.h"
#include "scene/Light.h"
#include "render/RayTracer.h"

struct Tile
{
    int startX;
    int startY;
    int endX;
    int endY;
};

class Renderer
{
public:
    Renderer(int width, int height);
    ~Renderer();

    void render(const Camera& camera, const BVH& bvh, const std::vector<Light>& lights);

    const std::uint32_t* getPixels() const;
    int getWidth() const;
    int getHeight() const;

    void toggleSmoothShading();
    bool isSmoothShadingEnabled() const;

private:
    static constexpr int TILE_SIZE = 32;

    int width;
    int height;
    std::vector<std::uint32_t> pixels;
    RayTracer rayTracer;
    bool smoothShadingEnabled = false;

    std::vector<std::thread> threads;
    std::queue<Tile> tileQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::condition_variable renderComplete;
    std::atomic<bool> stopPool;
    std::atomic<int> activeTiles;

    const Camera* currentCamera;
    const BVH* currentBVH;
    const std::vector<Light>* currentLights;

    void workerLoop();
    void renderPixel(const Camera& camera, const BVH& bvh, int x, int y);
    void renderTile(const Camera& camera, const BVH& bvh, int startX, int startY, int endX, int endY);
};
