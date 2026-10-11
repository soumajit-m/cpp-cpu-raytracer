#include "core/Camera.h"
#include <cmath>

namespace
{
    constexpr float PI = 3.14159265358979323846f;

    float degreesToRadians(float degrees)
    {
        return degrees * PI / 180.0f;
    }
}

Camera::Camera()
    : position(0.0f, 0.0f, 6.0f),
    forward(0.0f, 0.0f, -1.0f),
    right(1.0f, 0.0f, 0.0f),
    up(0.0f, 1.0f, 0.0f),
    yaw(-90.0f),
    pitch(0.0f),
    fov(72.0f),
    aspectRatio(16.0f / 9.0f)
{
    updateVectors();
}

const Vec3& Camera::getPosition() const
{
    return position;
}

const Vec3& Camera::getForward() const
{
    return forward;
}

const Vec3& Camera::getRight() const
{
    return right;
}

const Vec3& Camera::getUp() const
{
    return up;
}

void Camera::moveForward(float amount)
{
    position += forward * amount;
}

void Camera::moveRight(float amount)
{
    position += right * amount;
}

void Camera::moveUp(float amount)
{
    position += up * amount;
}

void Camera::rotate(float yawDelta, float pitchDelta)
{
    yaw += yawDelta;
    pitch += pitchDelta;

    // Prevent the camera from flipping upside down.
    constexpr float MAX_PITCH = 89.0f;

    if (pitch > MAX_PITCH)
    {
        pitch = MAX_PITCH;
    }

    if (pitch < -MAX_PITCH)
    {
        pitch = -MAX_PITCH;
    }

    updateVectors();
}

void Camera::setAspectRatio(float newAspectRatio)
{
    aspectRatio = newAspectRatio;
}

void Camera::setFov(float newFov)
{
    fov = newFov;
}

void Camera::updateVectors()
{
    const float yawRadians = degreesToRadians(yaw);
    const float pitchRadians = degreesToRadians(pitch);

    forward.x = std::cos(yawRadians) * std::cos(pitchRadians);
    forward.y = std::sin(pitchRadians);
    forward.z = std::sin(yawRadians) * std::cos(pitchRadians);
    forward = forward.normalized();

    right = Vec3(0.0f, 1.0f, 0.0f).cross(forward).normalized();
    up = forward.cross(right).normalized();

}

float Camera::getFov() const
{
    return fov;
}