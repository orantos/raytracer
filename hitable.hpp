#ifndef HITABLEHPP
#define HITABLEHPP

#include "ray.hpp"
#include "aabb.hpp"

class material;

/*
 * Helper function to compute the UV coordinates for a sphere on a hitpoint p.
 * For more information check episode 5 on the second book, "Image texture mapping".
 */
void get_sphere_uv(const vec3 &p, float &u, float &v)
{
    float phi = atan2(p.z(), p.x());
    float theta = asin(p.y());

    u = 1 - (phi + M_PI) / (2 * M_PI);
    v = (theta + M_PI / 2) / M_PI;
}

struct hit_record
{
    float t; // "t" in p(t) = A + t*B, for the ray. The hit will only count if "t" is between a given t_min and t_max.
    float u; // For texture mapping.
    float v; // For texture mapping.
    vec3 p; // Position of the hit
    vec3 normal;  // Normal of the hit. For a spher eit would be p - center of the sphere.
    material *mat_ptr;  // Material properties of the "hitable"
};

class hitable
{
    public:
        /*
         * t_min and t_max: The hit will only count if t_min < t < t_max
         */
        virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const = 0;
        virtual bool bounding_box(float t0, float t1, aabb &box) const = 0;
};

#endif // HITABLEHPP
