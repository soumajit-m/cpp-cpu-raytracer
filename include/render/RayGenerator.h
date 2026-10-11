#pragma once

#include "core/Camera.h"
#include "core/Ray.h"

class RayGenerator
{
public:
    static Ray generate(
        const Camera& camera,
        int pixelX,
        int pixelY,
        int width,
        int height
    );
};
