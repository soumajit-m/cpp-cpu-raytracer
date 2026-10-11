#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <string>
#include <vector>

#include "core/Camera.h"
#include "scene/Sphere.h"
#include "scene/BVH.h"
#include "render/Renderer.h"
#include "scene/Material.h" // Phase 2
#include "scene/Light.h"    // Phase 2

int main()
{
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr int RENDER_WIDTH = 1280;
    constexpr int RENDER_HEIGHT = 720;
    constexpr float MOVE_SPEED = 5.0f;
    constexpr float MOUSE_SENSITIVITY = 0.1f;
    constexpr int FONT_SIZE = 24;

    std::cout << "Starting renderer...\n";

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    if (!TTF_Init())
    {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CPU Ray Tracer", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (window == nullptr)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(window, nullptr);
    if (sdlRenderer == nullptr)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(
        sdlRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH,
        RENDER_HEIGHT
    );

    if (texture == nullptr)
    {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(sdlRenderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", FONT_SIZE);
    if (font == nullptr)
    {
        std::cerr << "TTF_OpenFont failed: " << SDL_GetError() << '\n';
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(sdlRenderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    if (!SDL_SetWindowRelativeMouseMode(window, true))
    {
        std::cerr << "Warning: Could not enable relative mouse mode: " << SDL_GetError() << '\n';
    }

    Camera camera;
    camera.setAspectRatio(static_cast<float>(RENDER_WIDTH) / static_cast<float>(RENDER_HEIGHT));

    Renderer renderer(RENDER_WIDTH, RENDER_HEIGHT);

    // =========================================================
    // SCENE: Materials, Geometry, Lights, & BVH
    // =========================================================
    std::cout << "Setting up scene...\n";

    // 1. Define Materials
    Material floorMat(Vec3(0.8f, 0.8f, 0.8f), 0.1f, 0.9f, 0.1f, 8.0f);      // Diffuse gray
    Material redPlastic(Vec3(1.0f, 0.2f, 0.2f), 0.1f, 0.7f, 0.9f, 64.0f);   // Shiny red
    Material blueMatte(Vec3(0.2f, 0.4f, 1.0f), 0.1f, 0.9f, 0.2f, 16.0f);    // Soft matte blue

    // 2. Generate Geometry
    std::vector<Triangle> sceneTriangles;

// --- FIX 2: TESSELLATE THE FLOOR ---
    constexpr int GRID_SIZE = 5;
    constexpr float TILE_SIZE = 5.0f;
    constexpr float OFFSET = (GRID_SIZE * TILE_SIZE) / 2.0f;

    for (int x = 0; x < GRID_SIZE; ++x)
    {
        for (int z = 0; z < GRID_SIZE; ++z)
        {
            float px = (x * TILE_SIZE) - OFFSET;
            float pz = (z * TILE_SIZE) - OFFSET;
            
            Vec3 v0(px, -1.0f, pz + TILE_SIZE);
            Vec3 v1(px + TILE_SIZE, -1.0f, pz + TILE_SIZE);
            Vec3 v2(px + TILE_SIZE, -1.0f, pz);
            Vec3 v3(px, -1.0f, pz);
            Vec3 n(0.0f, 1.0f, 0.0f);

            sceneTriangles.push_back(Triangle(v0, v1, v2, n, n, n, floorMat));
            sceneTriangles.push_back(Triangle(v0, v2, v3, n, n, n, floorMat));
        }
    }

    // Sphere 1 (Red Plastic, sitting on the floor)
    auto sphere1 = Sphere::generateMesh(Vec3(0.0f, 0.0f, -1.0f), 1.0f, 10, 10, redPlastic);
    sceneTriangles.insert(sceneTriangles.end(), sphere1.begin(), sphere1.end());

    // Sphere 2 (Blue Matte, sitting slightly closer on the floor)
    //auto sphere2 = Sphere::generateMesh(Vec3(1.5f, 0.0f, 1.0f), 1.0f, 10, 10, blueMatte);
    //sceneTriangles.insert(sceneTriangles.end(), sphere2.begin(), sphere2.end());

    // 3. Build BVH
    std::cout << "Building BVH with " << sceneTriangles.size() << " triangles...\n";
    BVH sceneBVH(sceneTriangles);
    std::cout << "BVH Build complete.\n";

    // 4. Setup Lighting Rig
    std::vector<Light> lights;
    // Main directional sunlight casting strong shadows
    //lights.push_back(Light::Directional(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 0.95f, 0.9f), 1.0f));
    // Warm point light acting as a rim/fill light from the right
    lights.push_back(Light::Point(Vec3(4.0f, 3.0f, 1.0f), Vec3(1.0f, 1.0f, 0.8f), 2.0f));

    using Clock = std::chrono::steady_clock;
    auto previousTime = Clock::now();
    double fpsTimer = 0.0;
    int frameCount = 0;
    double displayedFPS = 0.0;
    double displayedFrameTime = 0.0;
    bool running = true;

    while (running)
    {
        auto currentTime = Clock::now();
        std::chrono::duration<float> elapsed = currentTime - previousTime;
        previousTime = currentTime;
        float deltaTime = elapsed.count();
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_ESCAPE) running = false;
                if (event.key.key == SDLK_M) renderer.toggleSmoothShading();
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION)
            {
                camera.rotate(event.motion.xrel * MOUSE_SENSITIVITY, -event.motion.yrel * MOUSE_SENSITIVITY);
            }
        }

        const bool* keyboard = SDL_GetKeyboardState(nullptr);
        float movement = MOVE_SPEED * deltaTime;
        if (keyboard[SDL_SCANCODE_W]) camera.moveForward(movement);
        if (keyboard[SDL_SCANCODE_S]) camera.moveForward(-movement);
        if (keyboard[SDL_SCANCODE_A]) camera.moveRight(-movement);
        if (keyboard[SDL_SCANCODE_D]) camera.moveRight(movement);
        if (keyboard[SDL_SCANCODE_SPACE]) camera.moveUp(movement);
        if (keyboard[SDL_SCANCODE_LCTRL]) camera.moveUp(-movement);

        auto renderStart = Clock::now();
        
        // --- PHASE 2 RENDER CALL ---
        renderer.render(camera, sceneBVH, lights);
        
        auto renderEnd = Clock::now();
        std::chrono::duration<double, std::milli> renderDuration = renderEnd - renderStart;

        SDL_UpdateTexture(
            texture, nullptr, renderer.getPixels(),
            renderer.getWidth() * static_cast<int>(sizeof(std::uint32_t))
        );

        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderTexture(sdlRenderer, texture, nullptr, nullptr);

        fpsTimer += elapsed.count();
        frameCount++;
        if (fpsTimer >= 1.0)
        {
            displayedFPS = static_cast<double>(frameCount) / fpsTimer;
            displayedFrameTime = 1000.0 / displayedFPS;
            fpsTimer = 0.0;
            frameCount = 0;
        }

        std::string fpsText = "FPS: " + std::to_string(displayedFPS);
        std::string frameText = "Frame: " + std::to_string(displayedFrameTime) + " ms";

        SDL_Surface* fpsSurface = TTF_RenderText_Blended(font, fpsText.c_str(), 0, SDL_Color{255, 255, 255, 255});
        if (fpsSurface != nullptr)
        {
            SDL_Texture* fpsTex = SDL_CreateTextureFromSurface(sdlRenderer, fpsSurface);
            if (fpsTex != nullptr)
            {
                SDL_FRect dest{10.0f, 10.0f, static_cast<float>(fpsSurface->w), static_cast<float>(fpsSurface->h)};
                SDL_RenderTexture(sdlRenderer, fpsTex, nullptr, &dest);
                SDL_DestroyTexture(fpsTex);
            }
            SDL_DestroySurface(fpsSurface);
        }

        SDL_Surface* frameSurface = TTF_RenderText_Blended(font, frameText.c_str(), 0, SDL_Color{255, 255, 255, 255});
        if (frameSurface != nullptr)
        {
            SDL_Texture* frameTex = SDL_CreateTextureFromSurface(sdlRenderer, frameSurface);
            if (frameTex != nullptr)
            {
                SDL_FRect dest{10.0f, 40.0f, static_cast<float>(frameSurface->w), static_cast<float>(frameSurface->h)};
                SDL_RenderTexture(sdlRenderer, frameTex, nullptr, &dest);
                SDL_DestroyTexture(frameTex);
            }
            SDL_DestroySurface(frameSurface);
        }

        SDL_RenderPresent(sdlRenderer);
    }

    std::cout << "Renderer shutting down...\n";
    SDL_SetWindowRelativeMouseMode(window, false);
    TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    std::cout << "Renderer shut down successfully.\n";

    return 0;
}