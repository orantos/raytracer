#ifndef ORTHONORMALHPP
#define ORTHONORMALHPP

#include "vec3.hpp"

/*
 * Class that converts from given (x, y, z) relative to a given axis (usually Z), to a surface normal vector
 */
class orthonormal
{
	public:
		orthonormal() {}

		inline vec3 operator[](int i) const
		{
			return axis[i];
		}

		vec3 u() const
		{
			return axis[0];
		}

		vec3 v() const
		{
			return axis[1];
		}

		vec3 w() const
		{
			return axis[2];
		}

		vec3 local(float a, float b, float c) const
		{
			return a * u() + b * v() + c * w();
		}

		vec3 local(const vec3 &a) const
		{
			return a.x() * u() + a.y() * v() + a.z() * w();
		}

		void build_from_w(const vec3 &n);

		vec3 axis[3];
};

/*
 * Get the vector using the right orthonormal base. For more information check chapter 6 of the third book.
 */
void orthonormal::build_from_w(const vec3 &n)
{
	axis[2] = unit_vector(n);

	vec3 a;

	/*
	 * Check whether a given vector is a particular axis, and if it is NOT, then use that axis. This is done to avoid
	 * getting/selecting a parallel one.
	*/
	if (fabs(w().x()) > 0.9)
	{
		a = vec3(0, 1, 0);
	}
	else
	{
		a = vec3(1, 0, 0);
	}

	axis[1] = unit_vector(cross(w(), a));
	axis[0] = cross(w(), v());
}

#endif // ORTHONORMALHPP