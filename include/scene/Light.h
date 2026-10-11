#pragma once
#include "math/Vec3.h"

enum class LightType { Point, Directional };

struct Light
{
    LightType type;
    Vec3 position;
    Vec3 direction;
    Vec3 color;
    float intensity;

    static Light Point(const Vec3& pos, const Vec3& col, float intensity = 1.0f) {
        Light l; l.type = LightType::Point; l.position = pos; l.color = col; l.intensity = intensity; return l;
    }

    static Light Directional(const Vec3& dir, const Vec3& col, float intensity = 1.0f) {
        Light l; l.type = LightType::Directional; l.direction = dir.normalized(); l.color = col; l.intensity = intensity; return l;
    }
};
