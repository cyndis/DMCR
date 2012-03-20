/*
 * This file is a part of the DMCR project and is subject to the terms and
 * conditions defined in file 'LICENSE.txt', which is a part of this source
 * code package.
 */

#include "sphere.h"
#include <algorithm>

dmcr::AABB dmcr::Sphere::aabb() const
{
    return dmcr::AABB::
            fromCenterAndExtents(dmcr::SceneObject::position(),
                                 dmcr::Vector3f(m_radius*2, m_radius*2, m_radius*2));
}

dmcr::IntersectionResult dmcr::Sphere::intersects(const dmcr::Ray & ray)
const
{
    dmcr::Vector3f ray_origin = ray.origin() - position();
    
    float a = ray.direction().dot(ray.direction());
    float b = 2 * ray.direction().dot(ray_origin);
    float c = ray_origin.dot(ray_origin) - m_radius;

    float dcrim = b * b - 4 * a * c;
    if (dcrim < 0.0f) {
        dmcr::IntersectionResult result;
        result.intersects = false;
        return result;
    }

    float a2inv = 1.0 / (2.0 * a);
    float t0 = (-b + dcrim) * a2inv;
    float t1 = (-b - dcrim) * a2inv;

    dmcr::IntersectionResult result;
    result.intersects = true;
    result.t = std::min(t0, t1);
    result.normal = (ray.origin() + result.t * ray.direction() - position()) / m_radius;
    
    return result;
}

