#ifndef TEXTUREHPP
#define TEXTUREHPP

#include "perlin.hpp"

class texture
{
	public:
		virtual vec3 value(float u, float v, const vec3 &p) const = 0;
};

class constant_texture : public texture
{
	public:
		constant_texture() {}
		constant_texture(vec3 c) : color(c) {}

		virtual vec3 value(float u, float v, const vec3 &p) const
		{
			return color;
		}

		vec3 color;
};

/*
 * Simple class to create a 3D checker pattern.
 *
 * This is achieved by noting that the sign of sine and cosine just
 * alternates in a regular way and if we multiply trig functions in all three dimensions, the sign of that product forms
 * a 3D checker pattern.
 */
class checker_texture : public texture
{
	public:
		checker_texture() {}
		checker_texture(texture *t0, texture *t1) : even(t0), odd(t1) {}

		virtual vec3 value(float u, float v, const vec3 &p) const
		{
			float sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());

			if (sines < 0)
			{
				return odd->value(u, v, p);
			}
			else
			{
				return even->value(u, v, p);
			}
		}

		texture *even;
		texture *odd;
};

class noise_texture : public texture
{
	public:
		noise_texture() {}
		noise_texture(float sc) : scale(sc) {}

		virtual vec3 value(float u, float v, const vec3 &p) const
		{
			// return vec3(1, 1, 1) * 0.5 * (1 + noise.noise(scale * p));
			//return vec3(1, 1, 1) * 0.5 * (1 + noise.turbulence(scale * p));
			//return vec3(1, 1, 1) * noise.turbulence(scale * p);
			return vec3(1, 1, 1) * 0.5 * (1 + sin(scale * p.z() + 10 * noise.turbulence(p)));
		}

		perlin noise;
		// Scale to decide how quick the noise should vary. In other words, this variale decide the noise frequency.
		float scale;
};

#endif // TEXTUREHPP