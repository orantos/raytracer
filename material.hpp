#ifndef MATERIALHPP
#define MATERIALHPP

#include "hitable.hpp"

float schlick(float cosine, float ref_idx)
{
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;

	return r0 + (1 - r0) * pow(1 - cosine, 5.0);
}

bool refract(const vec3 &v, const vec3 &n, float ni_over_nt, vec3 &refracted)
{
	// Snell's law vector form. Can be found on the wikipedia or the GLSL spec in Khronos.
	vec3 uv = unit_vector(v);
	float dt = dot(uv, n);
	float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);

	if (discriminant > 0)
	{
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else
	{
		return false;
	}
}

vec3 reflect(const vec3 &v, const vec3 &n)
{
	/* "n" is a unit vector.
	   "v" is an incoming vector.
	   "dot(v, n)" is the length (or cosine), from "v" to the normal. We multiple this by "n" to modify the normal
	   which is a unit vector. The point of this is to obtain a vector that is the reflection of "v".
	*/
	return v - 2.0 * dot(v, n) * n;
}

vec3 random_in_unit_sphere()
{
    vec3 p;

    do
    {
        p = 2.0 * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
    } while (p.squared_length() >= 1.0);

    return p;
}

class material
{
	public:
		virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const = 0;
};

class lambertian : public material
{
	public:
		lambertian(const vec3 &a) : albedo(a) {}

		virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
		{
			vec3 target = rec.p + rec.normal + random_in_unit_sphere();
			scattered = ray(rec.p,  target - rec.p, r_in.time());
			attenuation = albedo;

			return true;
		}

		vec3 albedo;
};

class metal : public material
{
	public:
		metal(const vec3 &a, float f) : albedo(a)
		{
			if (f < 1)
			{
				fuzz = f;
			}
			else
			{
				fuzz = 1;
			}
		}

		virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
		{
			vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
			scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
			attenuation = albedo;

			return (dot(scattered.direction(), rec.normal) > 0);
		}

		vec3 albedo;
		float fuzz;
};

class dielectric : public material
{
	public:
		dielectric(float ri) : ref_idx(ri) {}

		virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
		{
			vec3 outward_normal;
			vec3 reflected = reflect(r_in.direction(), rec.normal);
			vec3 refracted;
			attenuation = vec3(1.0, 1.0, 1.0);
			float ni_over_nt;
			float reflect_prob;
			float cosine;

			if (dot(r_in.direction(), rec.normal) > 0)
			{
				outward_normal = -rec.normal;
				ni_over_nt = ref_idx;
				cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
			}
			else
			{
				outward_normal = rec.normal;
				ni_over_nt = 1.0 / ref_idx;
				cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
			}

			if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
			{
				reflect_prob = schlick(cosine, ref_idx);
			}
			else
			{
				reflect_prob = 1.0;
			}

			if (drand48() < reflect_prob)
			{
				scattered = ray(rec.p, reflected);
			}
			else
			{
				scattered = ray(rec.p, refracted);
			}

			return true;
		}

		float ref_idx;
};

#endif /* MATERIALHPP */
