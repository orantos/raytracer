#ifndef AARECTHPP
#define AARECTHPP

#include "hitable.hpp"

// XY Axis-aligned rectangle class.
class xy_rect : public hitable
{
	public:
		xy_rect() {}
		xy_rect(float arg_x0, float arg_x1, float arg_y0, float arg_y1, float arg_k, material *m) :
		    x0(arg_x0),
			x1(arg_x1),
			y0(arg_y0),
			y1(arg_y1),
			k(arg_k),
			mat(m)
		{};

		virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const;

		virtual bool bounding_box(float t0, float t1, aabb &box) const
		{
			box = aabb(vec3(x0, y0, k - 0.0001), vec3(x1, y1, k + 0.0001));

			return true;
		}

		material *mat;
		// 4 points defining a rectangle/plane.
		float x0;
		float x1;
		float y0;
		float y1;
		// K value that defines the location of the plane.
		float k;
};

/*
 * Check chapter 6 of the second book to understand the logic behind this. It is just using p(t) = a + b*t, and using
 * the remaining component.
 */
bool xy_rect::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	float t = (k - r.origin().z()) / r.direction().z();

	if (t < t_min || t > t_max)
	{
		return false;
	}

	float x = r.origin().x() + t * r.direction().x();
	float y = r.origin().y() + t * r.direction().y();

	if (x < x0 || x > x1 || y < y0 || y > y1)
	{
		return false;
	}

	rec.u = (x - x0) / (x1 - x0);
	rec.v = (y - y0) / (y1 - y0);
	rec.t = t;
	rec.mat_ptr = mat;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(0, 0, 1);

	return true;
}

// XZ Axis-aligned rectangle class.
class xz_rect : public hitable
{
	public:
		xz_rect() {}
		xz_rect(float arg_x0, float arg_x1, float arg_z0, float arg_z1, float arg_k, material *m) :
			x0(arg_x0),
			x1(arg_x1),
		    z0(arg_z0),
			z1(arg_z1),
			k(arg_k),
			mat(m)
		{};

		virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const;

		virtual bool bounding_box(float t0, float t1, aabb &box) const
		{
			box = aabb(vec3(x0,k - 0.0001, z0), vec3(x1, k + 0.0001, z1));

			return true;
		}

		virtual float pdf_value(const vec3 &origin, const vec3 &v) const
        {
        	hit_record rec;

			if (this->hit(ray(origin, v), 0.001, FLT_MAX, rec))
			{
				float area = (x1 - x0) * (z1 - z0);
				float distance_squared = rec.t * rec.t *  v.squared_length();
				float cosine = fabs(dot(v, rec.normal) / v.length());

				return distance_squared / (cosine * area);
			}
			else
			{
				return 0;
			}
        }

        virtual vec3 random(const vec3 &origin) const
		{
			vec3 random_point = vec3(x0 + drand48() * (x1 - x0), k, z0 + drand48() * (z1 - z0));

			return random_point - origin;
		}

		material *mat;
		// 4 points defining a rectangle/plane.
		float x0;
		float x1;
		float z0;
		float z1;
		// K value that defines the location of the plane.
		float k;
};

/*
 * Check chapter 6 of the second book to understand the logic behind this. It is just using p(t) = a + b*t, and using
 * the remaining component.
 */
bool xz_rect::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	float t = (k - r.origin().y()) / r.direction().y();

	if (t < t_min || t > t_max)
	{
		return false;
	}

	float x = r.origin().x() + t * r.direction().x();
	float z = r.origin().z() + t * r.direction().z();

	if (x < x0 || x > x1 || z < z0 || z > z1)
	{
		return false;
	}

	rec.u = (x - x0) / (x1 - x0);
	rec.v = (z - z0) / (z1 - z0);
	rec.t = t;
	rec.mat_ptr = mat;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(0, 1, 0);

	return true;
}

// YZ Axis-aligned rectangle class.
class yz_rect : public hitable
{
	public:
		yz_rect() {}
		yz_rect(float arg_y0, float arg_y1, float arg_z0, float arg_z1, float arg_k, material *m) :
			y0(arg_y0),
			y1(arg_y1),
		    z0(arg_z0),
			z1(arg_z1),
			k(arg_k),
			mat(m)
		{};

		virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const;

		virtual bool bounding_box(float t0, float t1, aabb &box) const
		{
			box = aabb(vec3(k - 0.0001, y0, z0), vec3(k + 0.0001, y1, z1));

			return true;
		}

		material *mat;
		// 4 points defining a rectangle/plane.
		float y0;
		float y1;
		float z0;
		float z1;
		// K value that defines the location of the plane.
		float k;
};

/*
 * Check chapter 6 of the second book to understand the logic behind this. It is just using p(t) = a + b*t, and using
 * the remaining component.
 */
bool yz_rect::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	float t = (k - r.origin().x()) / r.direction().x();

	if (t < t_min || t > t_max)
	{
		return false;
	}

	float y = r.origin().y() + t * r.direction().y();
	float z = r.origin().z() + t * r.direction().z();

	if (y < y0 || y > y1 || z < z0 || z > z1)
	{
		return false;
	}

	rec.u = (y - y0) / (y1 - y0);
	rec.v = (z - z0) / (z1 - z0);
	rec.t = t;
	rec.mat_ptr = mat;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(1, 0, 0);

	return true;
}

#endif // AARECTHPP