/*
 * This file is a part of the DMCR project and is subject to the terms and
 * conditions defined in file 'LICENSE.txt', which is a part of this source
 * code package.
 */

#include "aabb.h"
#include <cmath>
#include <algorithm>

#define EPSILON 0.000000001

dmcr::AABB::AABB(const dmcr::Vector3f &min, const dmcr::Vector3f &max) :
    m_min(min), m_max(max)
{
}

bool dmcr::AABB::intersects(const dmcr::AABB &aabb) const
{
    if (aabb.min().x() > m_max.x())
        return false;
    if (aabb.min().y() > m_max.y())
        return false;
    if (aabb.min().z() > m_max.z())
        return false;
    if (aabb.max().x() < m_min.x())
        return false;
    if (aabb.max().y() < m_min.y())
        return false;
    if (aabb.max().z() < m_min.z())
        return false;
    return true;
}

// Test ray and AABB intersection using slabs method.
double dmcr::AABB::intersects(const dmcr::Ray &ray) const
{
    dmcr::Vector3f ray_dir_inv = dmcr::Vector3f(1, 1, 1) / ray.direction();
    dmcr::Vector3f t1 = (m_min - ray.origin()) * ray_dir_inv;
    dmcr::Vector3f t2 = (m_max - ray.origin()) * ray_dir_inv;

    double tmin = std::max(std::min(t1.z(), t2.z()), 
            std::max(std::min(t1.y(), t2.y()), std::min(t1.x(), t2.x())));
    double tmax = std::min(std::max(t1.z(), t2.z()), 
            std::min(std::max(t1.y(), t2.y()), std::max(t1.x(), t2.x())));

    if (tmax >= std::max(0.0, tmin))
        return tmax;
    return -1.0;
}

dmcr::AABB dmcr::AABB::fromCenterAndExtents(const dmcr::Vector3f &center,
                                            const dmcr::Vector3f &extents)
{
    return dmcr::AABB(center - extents / 2.0, center + extents / 2.0);
}

dmcr::AABB dmcr::AABB::fromOppositeCorners(const dmcr::Vector3f& corner1,
                                           const dmcr::Vector3f& corner2)
{
    dmcr::Vector3f min = corner1;
    dmcr::Vector3f max = corner1;
    
    if (corner2.x() < min.x())
        min.setX(corner2.x());
    if (corner2.y() < min.y())
        min.setY(corner2.y());
    if (corner2.z() < min.z())
        min.setZ(corner2.z());
    
    if (corner2.x() > max.x())
        max.setX(corner2.x());
    if (corner2.y() > max.y())
        max.setY(corner2.y());
    if (corner2.z() > max.z())
        max.setZ(corner2.z());
    
    return dmcr::AABB(min, max);
}
