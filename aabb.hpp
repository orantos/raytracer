#ifndef AABBHPP
#define AABBHPP

#include "ray.hpp"

// Axis-aligned bounding box class. Check whether the T intervals in the slabs overlap.
class aabb
{
	public:
		aabb() {}
		aabb(const vec3 &min, const vec3 &max)
		{
			m_tmin = min;
			m_tmax = max;
		}

		vec3 min() const
		{
			return m_tmin;
		}

		vec3 max() const
		{
			return m_tmax;
		}

		bool hit(const ray &r, float tmin, float tmax)
		{
			for (int a = 0; a < 3; a++)
			{
				float invD = 1.0f / r.direction()[a]; // avoid a division. Multiplication is faster.
				float t0 = (min()[a] - r.origin()[a]) * invD;
				float t1 = (max()[a] - r.origin()[a]) * invD;

				// If the ray is travelling in the negative direction.
				if (invD < 0.0f)
				{
					std::swap(t0, t1);
				}

				tmin = t0 > tmin ? t0 : tmin;
				tmax = t0 < tmax ? t0 : tmax;
			}
		}

		vec3 m_tmin;
		vec3 m_tmax;
};

aabb sorrounding_box(aabb box0, aabb box1)
{
	vec3 small(fmin(box0.min().x(), box1.min().x()),
	           fmin(box0.min().y(), box1.min().y()),
			   fmin(box0.min().z(), box1.min().z()));

	vec3 big(fmax(box0.min().x(), box1.min().x()),
	         fmax(box0.min().y(), box1.min().y()),
			 fmax(box0.min().z(), box1.min().z()));;

	return aabb(small, big);
}

#endif // AABB