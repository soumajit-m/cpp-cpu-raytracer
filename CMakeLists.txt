cmake_minimum_required(VERSION 3.20)
project(Renderer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Dependencies (Fixed uppercase package names)
# ============================================================
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

# ============================================================
# Executable
# ============================================================
add_executable(renderer
    src/main.cpp
    src/math/Vec3.cpp
    src/math/AABB.cpp
    src/core/Ray.cpp
    src/core/Camera.cpp
    src/scene/Triangle.cpp
    src/scene/Sphere.cpp
    src/scene/BVH.cpp
    src/render/RayTracer.cpp
    src/render/RayGenerator.cpp
    src/render/Renderer.cpp
)

# ============================================================
# Compiler Flags (Hardware SIMD Support)
# ============================================================
if(MSVC)
    target_compile_options(renderer PRIVATE /arch:AVX2 /O2)
endif()

# ============================================================
# Include directories
# ============================================================
target_include_directories(renderer PRIVATE include)

# ============================================================
# Libraries (Fixed correct vcpkg target names)
# ============================================================
target_link_libraries(renderer
    PRIVATE
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
)

# ============================================================
# Copy vcpkg DLLs (Handles both Debug and Release automatically)
# ============================================================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VCPKG_BIN "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/debug/bin")
else()
    set(VCPKG_BIN "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/bin")
endif()

add_custom_command(TARGET renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${VCPKG_BIN}"
        "$<TARGET_FILE_DIR:renderer>"
)