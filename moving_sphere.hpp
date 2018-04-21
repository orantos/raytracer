#ifndef MOVINGSPHEREHPP
#define MOVINGSPHEREHPP

#include "hitable.hpp"

class moving_sphere: public hitable
{
    public:
        moving_sphere() {}
        moving_sphere(vec3 cen0, vec3 cen1, float t0, float t1, float r, material *m) : center0(cen0), center1(cen1),
		              time0(t0), time1(t1), radius(r), mat_ptr(m) {};

		virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const;

        vec3 center(float time) const;
		vec3 center0; // Center coords at shutter open time.
		vec3 center1; // Center coords at shutter close time.
		float time0; // Shutter open time.
		float time1; // Shutter close time.
        float radius;
        material *mat_ptr;
};

/*
 * @brief Calculate a moving center that moves linearly from center0 at time0 to center1 at time1 for a given time.
 */
vec3 moving_sphere::center(float time) const
{
	return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

// Pretty muchh as regular hit method member in "sphere" class, but replacing "center" with center(r.time())
bool moving_sphere::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
    vec3 oc = r.origin() - center(r.time());
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;

    if (discriminant > 0)
    {
        float temp = (-b - sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min)
        {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center(r.time())) / radius;
            rec.mat_ptr = mat_ptr;

            return true;
        }

        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min)
        {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center(r.time())) / radius;
            rec.mat_ptr = mat_ptr;

            return true;
        }
    }
    return false;
}

#endif // MOVINGSPHEREHPP