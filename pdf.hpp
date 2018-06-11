#ifndef PDFCPP
#define PDFCPP

#include "orthonormal.hpp"

/*
 * Generate random directions over the hemisphere following a uniform distribution.
 */
static vec3 random_cosine_direction()
{
	float r1 = drand48();
	float r2 = drand48();

	float z = sqrt(1 - r2);
	float phi = 2 * M_PI * r1;
	float x = cos(phi) * 2 * sqrt(r2);
	float y = sin(phi) * 2 * sqrt(r2);

	return vec3(x, y, z);
}

class pdf
{
	public:
		virtual float value(const vec3 &direction) const = 0;
		virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf
{
	public:
		cosine_pdf(const vec3 &w)
		{
			uvw.build_from_w(w);
		}

		virtual float value(const vec3 &direction) const
		{
			float cosine = dot(unit_vector(direction), uvw.w());

			if (cosine > 0)
			{
				return cosine / M_PI;
			}
			else
			{
				return 0;
			}
		}

		virtual vec3 generate() const
		{
			return uvw.local(random_cosine_direction());
		}

		orthonormal uvw;
};

class hitable_pdf : public pdf
{
	public:
		hitable_pdf(hitable *p, const vec3 &o) : ptr(p), origin(o) {}

		virtual float value(const vec3 &direction) const
		{
			return ptr->pdf_value(origin, direction);
		}

		virtual vec3 generate() const
		{
			return ptr->random(origin);
		}

		vec3 origin;
		hitable *ptr;
};

class mixture_pdf : public pdf
{
	public:
		mixture_pdf(pdf *p0, pdf *p1)
		{
			mixed_pdfs[0] = p0;
			mixed_pdfs[1] = p1;
		}

		virtual float value(const vec3 &direction) const
		{
			return 0.5 * mixed_pdfs[0]->value(direction) + 0.5 * mixed_pdfs[1]->value(direction);
		}

		virtual vec3 generate() const
		{
			if (drand48() < 0.5)
			{
				return mixed_pdfs[0]->generate();
			}
			else
			{
				return mixed_pdfs[1]->generate();
			}
		}

		pdf *mixed_pdfs[2];
};

#endif // PDFCPP