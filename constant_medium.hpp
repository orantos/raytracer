#ifndef CONSTANTMEDIUMHPP
#define CONSTANTMEDIUMHPP

#include "hitable.hpp"
#include "float.h"

/*
 * Class to implement volumes. Look at chapter 8 for more information about the maths behind this class.
 */
class constant_medium : public hitable
{
	public:
		constant_medium(hitable *b, float d, texture *a) : boundary(b), density(d)
		{
			phase_function = new isotropic(a);
		}

		virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const;

        virtual bool bounding_box(float t0, float t1, aabb &box) const
		{
			return boundary->bounding_box(t0, t1, box);
		}

		hitable *boundary;
		float density;
		material *phase_function;
};

/*
 * "The probablity that a ray scatters in any small distance dL is: probabilty = C * dL.
 *
 * Where C is proportional to the optical density of the volume. if you go through all the differential equations, for
 * a random number you get a distance where the scatteing occurs. If that distance is outside the volume, then there is
 * no HIT. For a constant volume we just need the density C and the boundary"
 */
bool constant_medium::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	bool db = (drand48() < 0.00001);
	db = false; // This makes the line above Dead code as it is. Used to make sure that we dont hit error messages.

	hit_record rec1;
	hit_record rec2;

	if (boundary->hit(r, -FLT_MAX, FLT_MAX, rec1))
	{
		if (boundary->hit(r, rec1.t + 0.0001, FLT_MAX, rec2))
		{
			if (db)
			{
				std::cerr << "\nt0 t1 " << rec1.t << " " << rec2.t << "\n";
			}

			if (rec1.t < t_min)
			{
				rec1.t = t_min;
			}

			if (rec2.t > t_max)
			{
				rec2.t = t_max;
			}

			if (rec1.t > rec2.t)
			{
				return false;
			}

			if (rec1.t < 0)
			{
				rec1.t = 0;
			}

			float distance_inside_boundary = (rec2.t - rec1.t) * r.direction().length();
			float hit_distance = -(1 / density) * log(drand48());

			if (hit_distance < distance_inside_boundary)
			{
				if (db)
				{
					std::cerr << "hit_distance =  " << hit_distance << "\n";
				}

				rec.t = rec1.t + hit_distance / r.direction().length();

				if (db)
				{
					std::cerr << "rec.t =  " << rec.t << "\n";
				}

				rec.p = r.point_at_parameter(rec.t);

				if (db)
				{
					std::cerr << "rec.p =  " << rec.p << "\n";
				}

				rec.normal = vec3(1, 0, 0); // Arbitrary
				rec.mat_ptr = phase_function;

				return true;
			}
		}
	}

	return false;
}

#endif // CONSTANTMEDIUMHPP