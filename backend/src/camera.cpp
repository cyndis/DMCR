#include "camera.h"
#include <cmath>

dmcr::Ray dmcr::Camera::ray(float x, float y) const
{
    /*
     * TODO
     * Implement me properly! (see issue #10)
     *
     *Pseudocode here:
     at = normalize(self.look_at)
     up = vector3(???)
     horizontal = normalize(cross(at,up))
     vertical       = cross(horizontal,at)
     H = tan(0.5 * FOV)
     horizontal = H * horizontal
     vertical = (1 / ASPECT) * H * vertical
     */
    dmcr::Vector3f at = (m_look_at - m_position).normalized();
    dmcr::Vector3f up(0,1,0);
    dmcr::Vector3f horizontal = at.cross(up).normalized();
    dmcr::Vector3f vertical = horizontal.cross(at);
    float H = tan(0.5 * m_fov * 180/ M_PI);
    horizontal = horizontal * H;
    vertical = vertical * (1 / m_aspect) * H;
    return Ray(m_position, at + horizontal * (2*x-1) + vertical * (2*y-1));
    //return Ray(m_position, m_look_at - m_position); vanha
}
