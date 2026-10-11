#include "render/Renderer.h"
#include "render/RayGenerator.h"
#include <algorithm>
#include <cmath>

Renderer::Renderer(int width, int height)
    : width(width),
      height(height),
      pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)),
      stopPool(false),
      activeTiles(0),
      currentCamera(nullptr),
      currentBVH(nullptr),
      currentLights(nullptr)
{
    unsigned int hardwareThreads = std::thread::hardware_concurrency();
    if (hardwareThreads == 0) hardwareThreads = 4;
    for (unsigned int i = 0; i < hardwareThreads; ++i)
    {
        threads.emplace_back(&Renderer::workerLoop, this);
    }
}

Renderer::~Renderer()
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopPool = true;
    }
    condition.notify_all();
    for (std::thread& thread : threads)
    {
        if (thread.joinable()) thread.join();
    }
}

void Renderer::toggleSmoothShading()
{
    smoothShadingEnabled = !smoothShadingEnabled;
}

bool Renderer::isSmoothShadingEnabled() const
{
    return smoothShadingEnabled;
}

void Renderer::workerLoop()
{
    while (true)
    {
        Tile tile;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]() { return stopPool || !tileQueue.empty(); });
            if (stopPool && tileQueue.empty()) return;
            tile = tileQueue.front();
            tileQueue.pop();
        }
        renderTile(*currentCamera, *currentBVH, tile.startX, tile.startY, tile.endX, tile.endY);
        if (--activeTiles == 0) renderComplete.notify_one();
    }
}

void Renderer::renderPixel(const Camera& camera, const BVH& bvh, int x, int y)
{
    Ray ray = RayGenerator::generate(camera, x, y, width, height);
    RayTraceResult result = rayTracer.trace(ray, bvh, smoothShadingEnabled);

    if (!result.hit)
    {
        pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] = 0x2D2D2DFF;
        return;
    }

    Vec3 hitPoint = ray.at(result.distance);
    Vec3 normal = result.normal;
    Material mat = result.material;
    
    Vec3 finalColor(
        mat.color.x * mat.ambient,
        mat.color.y * mat.ambient,
        mat.color.z * mat.ambient
    );

    for (const Light& light : *currentLights)
    {
        Vec3 toLight;
        float distanceToLight;
        float attenuation = 1.0f;

        if (light.type == LightType::Point)
        {
            Vec3 lightVec = light.position - hitPoint;
            distanceToLight = lightVec.length();
            toLight = lightVec / distanceToLight;
            attenuation = 1.0f / (1.0f + 0.09f * distanceToLight + 0.032f * (distanceToLight * distanceToLight));
        }
        else
        {
            toLight = light.direction * -1.0f;
            distanceToLight = 999999.0f;
        }

        // --- FIX 1: THE BACKFACE CULL ---
        // If the surface faces away from the light, it is already in shadow.
        // We skip the expensive shadow ray and BVH traversal entirely!
        float diff = normal.dot(toLight);
        if (diff <= 0.0f) continue;

        //Ray shadowRay(hitPoint + normal * 0.001f, toLight);
        //bool inShadow = rayTracer.traceShadow(shadowRay, bvh, distanceToLight);
        //if (!inShadow)
        //{
            Vec3 viewDir = (camera.getPosition() - hitPoint).normalized();
            Vec3 halfwayDir = (toLight + viewDir).normalized();

            // diff is already calculated above and guaranteed positive
            float spec = std::pow(std::max(0.0f, normal.dot(halfwayDir)), mat.shininess);

            float lightInfluence = light.intensity * attenuation;
            finalColor.x += light.color.x * lightInfluence * (mat.color.x * mat.diffuse * diff + mat.specular * spec);
            finalColor.y += light.color.y * lightInfluence * (mat.color.y * mat.diffuse * diff + mat.specular * spec);
            finalColor.z += light.color.z * lightInfluence * (mat.color.z * mat.diffuse * diff + mat.specular * spec);
        //}
    }

    int r = static_cast<int>(std::clamp(finalColor.x * 255.0f, 0.0f, 255.0f));
    int g = static_cast<int>(std::clamp(finalColor.y * 255.0f, 0.0f, 255.0f));
    int b = static_cast<int>(std::clamp(finalColor.z * 255.0f, 0.0f, 255.0f));

    pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] = (r << 24) | (g << 16) | (b << 8) | 255u;
}

void Renderer::renderTile(const Camera& camera, const BVH& bvh, int startX, int startY, int endX, int endY)
{
    for (int y = startY; y < endY; ++y)
    {
        for (int x = startX; x < endX; ++x)
        {
            renderPixel(camera, bvh, x, y);
        }
    }
}

void Renderer::render(const Camera& camera, const BVH& bvh, const std::vector<Light>& lights)
{
    currentCamera = &camera;
    currentBVH = &bvh;
    currentLights = &lights;
    int tileCount = 0;

    {
        std::unique_lock<std::mutex> lock(queueMutex);
        for (int y = 0; y < height; y += TILE_SIZE)
        {
            for (int x = 0; x < width; x += TILE_SIZE)
            {
                tileQueue.push({x, y, std::min(x + TILE_SIZE, width), std::min(y + TILE_SIZE, height)});
                tileCount++;
            }
        }
        activeTiles = tileCount;
    }

    condition.notify_all();

    {
        std::unique_lock<std::mutex> lock(queueMutex);
        renderComplete.wait(lock, [this]() { return activeTiles == 0; });
    }
}

const std::uint32_t* Renderer::getPixels() const { return pixels.data(); }
int Renderer::getWidth() const { return width; }
int Renderer::getHeight() const { return height; }
