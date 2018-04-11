#ifndef CAMERAHPP
#define CAMERAHPP

#include "ray.hpp"

class camera
{
	public:
		// vfov is top to bottom in degrees
		camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect)
		{
			vec3 u, v, w;
			float theta = vfov * M_PI / 180;
			float half_height = tan(theta / 2);
			float half_width = aspect * half_height;

			origin = lookfrom;
			w = unit_vector(lookfrom - lookat);
			u = unit_vector(cross(vup, w));
			v = cross(w, u);

			/* width and height have this ratio to spot easily possible transpose issues */
			lower_left_corner = origin - half_width * u - half_height * v - w;
			horizontal = 2.0 * half_width * u;
			vertical = 2.0 * half_height * v;
		}

		ray get_ray(float u, float v)
		{
			return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
		}

		vec3 origin;
		vec3 lower_left_corner;
		vec3 horizontal;
		vec3 vertical;
};

#endif /* CAMERAHPP */
