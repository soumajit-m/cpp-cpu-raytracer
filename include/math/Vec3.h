#pragma once

#include <cmath>

struct Vec3
{
    float x;
    float y;
    float z;

    Vec3();
    Vec3(float x, float y, float z);

    float length() const;
    Vec3 normalized() const;

    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;
    Vec3& operator+=(const Vec3& other);
    Vec3& operator-=(const Vec3& other);

    float dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;
};
