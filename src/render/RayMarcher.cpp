#include "render/RayMarcher.h"

#include <limits>

RayMarchResult RayMarcher::march(
    const Ray& ray,
    const std::vector<Sphere>& spheres
) const
{
    float distanceTravelled = 0.0f;

    for (int step = 0; step < MAX_STEPS; ++step)
    {
        Vec3 position =
            ray.at(distanceTravelled);

        float closestDistance =
            std::numeric_limits<float>::max();

        int closestObject = -1;

        /*
            Find the closest object at this
            position along the ray.
        */
        for (int i = 0;
            i < static_cast<int>(spheres.size());
            ++i)
        {
            float distance =
                spheres[i].distance(position);

            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestObject = i;
            }
        }

        /*
            No objects exist in the scene.
        */
        if (closestObject == -1)
        {
            break;
        }

        /*
            Actual surface hit.
        */
        if (closestDistance < SURFACE_DISTANCE)
        {
            return {
                true,
                distanceTravelled,
                step + 1,
                closestObject
            };
        }

        /*
            SDF gives us a safe distance to travel
            before checking the scene again.
        */
        distanceTravelled += closestDistance;

        /*
            Ray has travelled beyond our useful
            rendering distance.
        */
        if (distanceTravelled > MAX_DISTANCE)
        {
            break;
        }
    }

    /*
        Ray did not hit an object.
    */
    return {
        false,
        distanceTravelled,
        MAX_STEPS,
        -1
    };
}