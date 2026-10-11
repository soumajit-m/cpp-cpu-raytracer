#include "render/Renderer_v1.h"
#include "render/RayGenerator.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <thread>
#include <vector>

Renderer::Renderer(
    int width,
    int height
)
    : width(width),
      height(height),
      pixels(
          static_cast<std::size_t>(width) *
          static_cast<std::size_t>(height)
      )
{
}

void Renderer::renderPixel(
    const Camera& camera,
    const std::vector<Sphere>& spheres,
    int x,
    int y,
    const Vec3& lightPosition
)
{
    Ray ray =
        RayGenerator::generate(
            camera,
            x,
            y,
            width,
            height
        );

    RayMarchResult result =
        rayMarcher.march(
            ray,
            spheres
        );

    std::uint32_t color;

    /*
        Ray hit an object.
    */
    if (result.hit)
    {
        Vec3 hitPoint =
            ray.at(result.distance);

        const Sphere& hitSphere =
            spheres[result.objectIndex];

        /*
            Sphere normal.
        */
        Vec3 normal =
            (hitPoint - hitSphere.center).normalized();

        /*
            Direction from surface to light.
        */
        Vec3 toLight =
            (lightPosition - hitPoint).normalized();

        /*
            Lambert lighting.
        */
        float brightness =
            normal.dot(toLight);

        brightness =
            std::max(
                0.0f,
                brightness
            );

        int intensity =
            static_cast<int>(
                brightness * 255.0f
            );

        intensity =
            std::clamp(
                intensity,
                0,
                255
            );

        /*
            RGBA.

            R = intensity
            G = intensity
            B = intensity
            A = 255
        */
        color =
            (static_cast<std::uint32_t>(intensity) << 24) |
            (static_cast<std::uint32_t>(intensity) << 16) |
            (static_cast<std::uint32_t>(intensity) << 8) |
            255u;
    }
    else
    {
        /*
            Background.
        */
        color = 0x2D2D2DFF;
    }

    pixels[
        static_cast<std::size_t>(y) *
        static_cast<std::size_t>(width) +
        static_cast<std::size_t>(x)
    ] = color;
}

float Renderer::calculateNearObjectDistance(
    float rayDistance
) const
{
    /*
        Convert ray distance into [0, 1].

            d = 0
                -> 0

            d = 100
                -> 1
    */
    float t =
        rayDistance /
        ADAPTIVE_MAX_DISTANCE;

    t =
        std::clamp(
            t,
            0.0f,
            1.0f
        );

    /*
        Cubic adaptive curve:

            threshold = (d / 100)^3

        This gives us a small threshold for
        nearby geometry and a larger threshold
        farther along the ray.
    */
    return std::pow(
        t,
        ADAPTIVE_EXPONENT
    );
}

bool Renderer::blockNeedsSubdivision(
    const Camera& camera,
    const std::vector<Sphere>& spheres,
    int startX,
    int startY,
    int endX,
    int endY
) const
{
    /*
        Four corner samples:

        A ---------------- B
        |                  |
        |      BLOCK       |
        |                  |
        C ---------------- D
    */

    const int x1 = startX;
    const int y1 = startY;

    const int x2 = endX - 1;
    const int y2 = endY - 1;

    const int sampleX[4] =
    {
        x1,
        x2,
        x1,
        x2
    };

    const int sampleY[4] =
    {
        y1,
        y1,
        y2,
        y2
    };

    for (int i = 0; i < 4; ++i)
    {
        /*
            Generate one corner ray.
        */
        Ray ray =
            RayGenerator::generate(
                camera,
                sampleX[i],
                sampleY[i],
                width,
                height
            );

        /*
            March the ray through the scene.
        */
        RayMarchResult result =
            rayMarcher.march(
                ray,
                spheres
            );

        /*
            RULE 1:

            If the corner ray actually hits
            geometry, this block needs more detail.
        */
        if (result.hit)
        {
            return true;
        }

        /*
            RULE 2:

            The ray missed the surface but may
            have passed close to an object.

            IMPORTANT:

            Use closestApproachDistance instead
            of result.distance.

            result.distance tells us how far the
            ray travelled before stopping.

            closestApproachDistance tells us where
            the ray actually came closest to geometry.
        */
        float adaptiveThreshold =
            calculateNearObjectDistance(
                result.closestApproachDistance
            );

        /*
            If the closest SDF distance is smaller
            than our threshold, subdivide.
        */
        if (result.closestSurfaceDistance <=
            adaptiveThreshold)
        {
            return true;
        }
    }

    /*
        None of the four corner rays hit or came
        sufficiently close to geometry.

        Treat the entire block as background.
    */
    return false;
}

void Renderer::fillBlock(
    int startX,
    int startY,
    int endX,
    int endY,
    std::uint32_t color
)
{
    for (int y = startY; y < endY; ++y)
    {
        for (int x = startX; x < endX; ++x)
        {
            pixels[
                static_cast<std::size_t>(y) *
                static_cast<std::size_t>(width) +
                static_cast<std::size_t>(x)
            ] = color;
        }
    }
}

void Renderer::renderBlock(
    const Camera& camera,
    const std::vector<Sphere>& spheres,
    int startX,
    int startY,
    int endX,
    int endY,
    const Vec3& lightPosition
)
{
    const int blockWidth =
        endX - startX;

    const int blockHeight =
        endY - startY;

    /*
        BASE CASE

        The block is exactly one pixel.

        We must actually render this pixel.
    */
    if (blockWidth == 1 &&
        blockHeight == 1)
    {
        renderPixel(
            camera,
            spheres,
            startX,
            startY,
            lightPosition
        );

        return;
    }

    /*
        Check the four corners.
    */
    if (!blockNeedsSubdivision(
            camera,
            spheres,
            startX,
            startY,
            endX,
            endY))
    {
        /*
            Nothing interesting was detected.

            Fill the whole block with background
            instead of calculating every pixel.
        */
        fillBlock(
            startX,
            startY,
            endX,
            endY,
            0x2D2D2DFF
        );

        return;
    }

    /*
        Something interesting was detected.

        Divide the block into four smaller blocks.
    */
    const int middleX =
        startX + blockWidth / 2;

    const int middleY =
        startY + blockHeight / 2;

    /*
        Top-left
    */
    if (startX < middleX &&
        startY < middleY)
    {
        renderBlock(
            camera,
            spheres,
            startX,
            startY,
            middleX,
            middleY,
            lightPosition
        );
    }

    /*
        Top-right
    */
    if (middleX < endX &&
        startY < middleY)
    {
        renderBlock(
            camera,
            spheres,
            middleX,
            startY,
            endX,
            middleY,
            lightPosition
        );
    }

    /*
        Bottom-left
    */
    if (startX < middleX &&
        middleY < endY)
    {
        renderBlock(
            camera,
            spheres,
            startX,
            middleY,
            middleX,
            endY,
            lightPosition
        );
    }

    /*
        Bottom-right
    */
    if (middleX < endX &&
        middleY < endY)
    {
        renderBlock(
            camera,
            spheres,
            middleX,
            middleY,
            endX,
            endY,
            lightPosition
        );
    }
}

void Renderer::render(
    const Camera& camera,
    const std::vector<Sphere>& spheres
)
{
    const Vec3 lightPosition(
        3.0f,
        4.0f,
        -2.0f
    );

    /*
        Initial grid:

            128 × 72 = 9216 blocks

        At 1280 × 720:

            10 × 10 pixels per block.
    */
    const int blockWidth =
        (width + GRID_COLUMNS - 1) /
        GRID_COLUMNS;

    const int blockHeight =
        (height + GRID_ROWS - 1) /
        GRID_ROWS;

    struct InitialBlock
    {
        int startX;
        int startY;
        int endX;
        int endY;
    };

    std::vector<InitialBlock> blocks;

    blocks.reserve(
        static_cast<std::size_t>(
            GRID_COLUMNS * GRID_ROWS
        )
    );

    /*
        Create the initial grid.
    */
    for (int gridY = 0;
         gridY < GRID_ROWS;
         ++gridY)
    {
        for (int gridX = 0;
             gridX < GRID_COLUMNS;
             ++gridX)
        {
            int startX =
                gridX * blockWidth;

            int startY =
                gridY * blockHeight;

            int endX =
                std::min(
                    startX + blockWidth,
                    width
                );

            int endY =
                std::min(
                    startY + blockHeight,
                    height
                );

            /*
                Ignore blocks outside the image.
            */
            if (startX >= width ||
                startY >= height)
            {
                continue;
            }

            blocks.push_back(
                {
                    startX,
                    startY,
                    endX,
                    endY
                }
            );
        }
    }

    /*
        Get the number of CPU hardware threads.
    */
    unsigned int hardwareThreads =
        std::thread::hardware_concurrency();

    if (hardwareThreads == 0)
    {
        hardwareThreads = 4;
    }

    /*
        Never create more threads than
        initial blocks.
    */
    unsigned int threadCount =
        std::min(
            hardwareThreads,
            static_cast<unsigned int>(
                blocks.size()
            )
        );

    /*
        Index of the next block that a worker
        should process.
    */
    std::atomic<std::size_t> nextBlock{0};

    /*
        Worker function.
    */
    auto worker =
        [&]()
        {
            while (true)
            {
                /*
                    Get a unique block index.

                    Each thread gets a different
                    initial block.
                */
                std::size_t index =
                    nextBlock.fetch_add(
                        1,
                        std::memory_order_relaxed
                    );

                /*
                    No blocks left.
                */
                if (index >= blocks.size())
                {
                    break;
                }

                const InitialBlock& block =
                    blocks[index];

                /*
                    Recursively render this block.
                */
                renderBlock(
                    camera,
                    spheres,
                    block.startX,
                    block.startY,
                    block.endX,
                    block.endY,
                    lightPosition
                );
            }
        };

    std::vector<std::thread> threads;

    threads.reserve(threadCount);

    /*
        Start workers.
    */
    for (unsigned int i = 0;
         i < threadCount;
         ++i)
    {
        threads.emplace_back(worker);
    }

    /*
        Wait for all workers to finish.
    */
    for (std::thread& thread : threads)
    {
        thread.join();
    }
}

const std::uint32_t* Renderer::getPixels() const
{
    return pixels.data();
}

int Renderer::getWidth() const
{
    return width;
}

int Renderer::getHeight() const
{
    return height;
}