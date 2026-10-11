#include "render/RayGenerator.h"
#include <cmath>

Ray RayGenerator::generate(
    const Camera& camera,
    int pixelX,
    int pixelY,
    int width,
    int height
)
{
    constexpr float PI = 3.14159265358979323846f;

    // Convert degrees to radians and calculate the half-height
    float fovRadians = camera.getFov() * PI / 180.0f;
    float halfHeight = std::tan(fovRadians * 0.5f);

    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    float halfWidth = halfHeight * aspectRatio;

    float u = (static_cast<float>(pixelX) + 0.5f) / static_cast<float>(width);
    float v = (static_cast<float>(pixelY) + 0.5f) / static_cast<float>(height);

    // Map screen coordinates from [0, 1] to [-1, 1]
    float screenX = 2.0f * u - 1.0f;
    float screenY = 1.0f - 2.0f * v;

    Vec3 direction =
        camera.getForward()
        + camera.getRight() * (screenX * halfWidth)
        + camera.getUp() * (screenY * halfHeight);

    return Ray(camera.getPosition(), direction.normalized());
}