#pragma once

#include "math/Vec3.h"

class Camera
{
public:
    Camera();

    // Position
    const Vec3& getPosition() const;

    // Orientation
    const Vec3& getForward() const;
    const Vec3& getRight() const;
    const Vec3& getUp() const;

    // Movement
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);

    // Rotation
    void rotate(float yawDelta, float pitchDelta);

    // Camera settings
    void setAspectRatio(float aspectRatio);
    void setFov(float fovDegrees);
    float getFov() const; 

private:
    Vec3 position;

    Vec3 forward;
    Vec3 right;
    Vec3 up;

    float yaw;
    float pitch;

    float fov;
    float aspectRatio;

    void updateVectors();
};
