#pragma once

#include <cstdint>
#include <vector>

#include "core/Camera.h"
#include "scene/Sphere.h"
#include "render/RayMarcher.h"

class Renderer
{
public:

    Renderer(
        int width,
        int height
    );

    void render(
        const Camera& camera,
        const std::vector<Sphere>& spheres
    );

    const std::uint32_t* getPixels() const;

    int getWidth() const;
    int getHeight() const;

private:


    static constexpr int GRID_COLUMNS = 96;
    static constexpr int GRID_ROWS = 54;

    /*
        Maximum ray distance used by the
        adaptive threshold function.
    */
    static constexpr float ADAPTIVE_MAX_DISTANCE = 100.0f;

    /*
        Adaptive threshold curve:

            T(d) = (d / 100)^3

        Example:

            d = 0
                -> 0

            d = 10
                -> 0.001

            d = 20
                -> 0.008

            d = 50
                -> 0.125

            d = 100
                -> 1.0

        This is a heuristic used to decide whether
        an adaptive block needs subdivision.
    */
    static constexpr float ADAPTIVE_EXPONENT = 4.0f;

    int width;
    int height;

    std::vector<std::uint32_t> pixels;

    RayMarcher rayMarcher;

    void renderPixel(
        const Camera& camera,
        const std::vector<Sphere>& spheres,
        int x,
        int y,
        const Vec3& lightPosition
    );

    float calculateNearObjectDistance(
        float rayDistance
    ) const;

    bool blockNeedsSubdivision(
        const Camera& camera,
        const std::vector<Sphere>& spheres,
        int startX,
        int startY,
        int endX,
        int endY
    ) const;

    void renderBlock(
        const Camera& camera,
        const std::vector<Sphere>& spheres,
        int startX,
        int startY,
        int endX,
        int endY,
        const Vec3& lightPosition
    );

    void fillBlock(
        int startX,
        int startY,
        int endX,
        int endY,
        std::uint32_t color
    );
};