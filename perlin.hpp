#ifndef PERLINHPP
#define PERLINHPP

#include "vec3.hpp"

/* Function to smooth out the Perlin generated noise using linear interpolation. In order to avoid "blockiness" on the
 * generated noise due to the min and max of the patttern landing exactly on the integer x/y/z, we use a trick from Ken
 * Perlin, to punt unit vectors (instead of floats) on the lattice points and use a dot product to move the min and max
 * off the lattice.
 */
inline float trilinear_interpolation(vec3 c[2][2][2], float u, float v, float w)
{
	float accumulation = 0;

	// Hermite Cubic interpolation to remove Mach Bands artifacts in the gnerated noise
	float uu = u * u * (3 - 2 * u);
	float vv = v * v * (3 - 2 * v);
	float ww = w * w * (3 - 2 * w);

	for(int i = 0; i < 2; i++)
	{
		for(int j = 0; j < 2; j++)
		{
			for(int k = 0; k < 2; k++)
			{
				vec3 weight_v(u - i, v - j, w - k);

				accumulation += (i*uu + (1-i) * (1-uu)) *
				                (j*vv + (1-j) * (1-vv)) *
								(k*ww + (1-k) * (1-ww)) * dot(c[i][j][k], weight_v);
			}
		}
	}

	return accumulation;
}

/*
 * Class to generate Perlin noise, which is an algorithm to generate "natural looking" textures. It takes a 3D point as
 * input and always return the same randomish number.
 */
class perlin
{
	public:
		float noise(const vec3 &p) const
		{
			float u = p.x() - floor(p.x());
			float v = p.y() - floor(p.y());
			float w = p.z() - floor(p.z());

			int i = floor(p.x());
			int j = floor(p.y());
			int k = floor(p.z());

			vec3 c[2][2][2];

			for(int di = 0; di < 2; di++)
			{
				for(int dj = 0; dj < 2; dj++)
				{
					for(int dk = 0; dk < 2; dk++)
					{
						c[di][dj][dk] = rand_vector[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];
					}
				}
			}

			return trilinear_interpolation(c, u, v, w);
		}

		float turbulence(const vec3 &p,  int depth = 7) const
		{
			float accumulation = 0;
			float weight = 1.0;
			vec3 temp_p = p;

			for(int i = 0; i < depth; i++)
			{
				accumulation += weight * noise(temp_p);
				weight *= 0.5;
				temp_p *= 2.0;
			}

			return fabs(accumulation);
		}

		static vec3 *rand_vector;
		static int *perm_x;
		static int *perm_y;
		static int *perm_z;
};

static vec3 * perlin_generate()
{
	vec3 *p = new vec3[256];
	for (int i = 0; i < 256; ++i)
	{
		p[i] = unit_vector(vec3( -1 + 2 * drand48(),  -1 + 2 * drand48(),  -1 + 2 * drand48()));
	}

	return p;
}

void permute(int *p, int n)
{
	for(int i = n-1; i > 0; i--)
	{
		int target = int(drand48() * (i + 1));
		int tmp = p[i];

		p[i] = p[target];
		p[target] = tmp;
	}

	return;
}

static int * perlin_generate_permute()
{
	int *p = new int[256];

	for(int i = 0; i < 256; i++)
	{
		p[i] = i;
	}

	permute(p, 256);

	return p;
}

vec3 *perlin::rand_vector = perlin_generate();
int *perlin::perm_x = perlin_generate_permute();
int *perlin::perm_y = perlin_generate_permute();
int *perlin::perm_z = perlin_generate_permute();

#endif // PERLINHPP