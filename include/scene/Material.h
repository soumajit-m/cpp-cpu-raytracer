#pragma once
#include "math/Vec3.h"

struct Material
{
    Vec3 color;
    float ambient;
    float diffuse;
    float specular;
    float shininess;

    Material(const Vec3& col = Vec3(0.8f, 0.8f, 0.8f), 
             float amb = 0.1f, float diff = 0.9f, 
             float spec = 0.5f, float shiny = 32.0f)
        : color(col), ambient(amb), diffuse(diff), specular(spec), shininess(shiny) {}
};
