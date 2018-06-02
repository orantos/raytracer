#ifndef MATERIALHPP
#define MATERIALHPP

#include "hitable.hpp"
#include "texture.hpp"
#include "orthonormal.hpp"

/*
 * Real glass has reflectivity that may varies with angle. This is a polynomial approximation done by Chritophe Schlick
 */
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

/*
 * This returns a random vector/point IN a unit sphere. The way it works is we pick a random point that is inside a
 * unit cube where X, Y and Z all range form -1 to +1. Then whe checker whether that point is also contained inside the
 * unit sphere contained inside the cube.
 */
vec3 random_in_unit_sphere()
{
    vec3 p;

    do
    {
        p = 2.0 * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
    } while (dot(p, p) >= 1.0);

    return p;
}

/*
 * This is a version of the function above but returns a unit vector, which in other words means a point/vector that is
 * on the surface of the unit sphere.
 */
vec3 random_on_unit_sphere()
{
	vec3 p = random_in_unit_sphere();

	return unit_vector(p);
}

class material
{
	public:
		virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &albedo, ray &scattered, float &pdf) const
		{
			return false;
		}

		virtual float scattering_pdf(const ray &ray_in, const hit_record &rec, ray &scattered) const
		{
			return 0.0;
		}

		// To avoid all the non-emitting materials to need to implement this function, we do it here in the base class.
		virtual vec3 emitted(float u, float v, const vec3 &p) const
		{
			return vec3(0, 0, 0);
		}
};

/*
 * Light emitting material (area lighting). Like the "background" in "main", it just tells the ray what color it is and
 * performs no reflection.
 */
class diffuse_light : public material
{
	public:
		diffuse_light(texture *a) : emit(a) {}

		virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
		{
			return false;
		}

		virtual vec3 emitted(float u, float v, const vec3 &p) const
		{
			return emit->value(u, v, p);
		}

		texture *emit;
};

static vec3 random_cosine_directions()
{
	float r1 = drand48();
	float r2 = drand48();

	float z = sqrt(1 - r2);
	float phi = 2 * M_PI * r1;
	float x = cos(phi) * 2 * sqrt(r2);
	float y = sin(phi) * 2 * sqrt(r2);

	return vec3(x, y, z);
}

class lambertian : public material
{
	public:
		lambertian(texture *a) : albedo(a) {}

		// the Scattering PDF of a Lambertian material is proportional to cos(theta), which is the dot product here
		float scattering_pdf(const ray &ray_in, const hit_record &rec, ray &scattered) const
		{
			float cosine = dot(rec.normal, unit_vector(scattered.direction()));

			if (cosine < 0)
			{
				cosine = 0;
			}

			return cosine / M_PI;
		}

		/*
		 * "scattter" is how much the ray has been dispersed, or say it absorvered the ray. If scattered, then
		 * "attenuation" represents how much the ray should be attenuated because of the scatter.
		 */
		virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &alb, ray &scattered, float &pdf) const
		{
			orthonormal uvw;

			uvw.build_from_w(rec.normal);

			vec3 direction = uvw.local(random_cosine_directions());

			scattered = ray(rec.p, unit_vector(direction), ray_in.time());
			alb = albedo->value(rec.u, rec.v, rec.p);

			// PDF to weight against.
			//pdf = dot(rec.normal, scattered.direction()) / M_PI;
			//pdf = 0.5 / M_PI;
			pdf = dot(uvw.w(), scattered.direction()) / M_PI;

			return true;
		}

		texture *albedo;
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

/*
 *  Class to implement isotropic materials. Look at chapter 8 for more information about the maths behind this class.
 */
class isotropic : public material
{
	public:
		isotropic(texture *a) : albedo(a) {}

		virtual bool scatter(const ray &ray_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
		{
			scattered = ray(rec.p, random_in_unit_sphere());
			attenuation = albedo->value(rec.u, rec.v, rec.p);

			return true;
		}

		texture *albedo;
};

#endif // MATERIALHPP
