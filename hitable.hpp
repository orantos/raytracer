#ifndef HITABLEHPP
#define HITABLEHPP

#include "ray.hpp"
#include "aabb.hpp"

class material;

struct hit_record
{
    float t; // "t" in p(t) = A + t*B, for the ray. The hit will only count if "t" is between a given t_min and t_max.
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
