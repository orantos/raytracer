#ifndef BVHHPP
#define BVHHPP

#include "hitable.hpp"

class bvh_node : public hitable
{
	public:
		bvh_node() {}
		bvh_node(hitable **list, int n, float time0, float time1);

		virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const;
		virtual bool bounding_box(float t0, float t1, aabb &box) const;

		hitable *left;
		hitable *right;
		aabb box;
};

bool bvh_node::bounding_box(float t0, float t1, aabb &b) const
{
	b = box;

	return true;
}

bool bvh_node::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	if (box.hit(r, t_min, t_max))
	{
		hit_record left_rec;
		hit_record right_rec;

		bool hit_left = left->hit(r, t_min, t_max, left_rec);
		bool hit_right = right->hit(r, t_min, t_max, right_rec);

		if (hit_left && hit_right)
		{
			if (left_rec.t < right_rec.t)
			{
				rec = left_rec;
			}
			else
			{
				rec = right_rec;
			}

			return true;
		}
		else if (hit_left)
		{
			rec = left_rec;
			return true;
		}
		else if (hit_right)
		{
			rec = right_rec;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

static int box_x_compare(const void *a, const void *b)
{
	aabb box_left;
	aabb box_right;
	hitable *ah = *(hitable **)a;
	hitable *bh = *(hitable **)b;

	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
	{
		std::cerr << "No bounding box in bvh_node constructor \n";
	}

	if (box_left.min().x() - box_right.min().x() < 0.0)
	{
		return - 1;
	}
	else
		return 1;
}

static int box_y_compare(const void *a, const void *b)
{
	aabb box_left;
	aabb box_right;
	hitable *ah = *(hitable **)a;
	hitable *bh = *(hitable **)b;

	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
	{
		std::cerr << "No bounding box in bvh_node constructor \n";
	}

	if (box_left.min().y() - box_right.min().y() < 0.0)
	{
		return - 1;
	}
	else
		return 1;
}

static int box_z_compare(const void *a, const void *b)
{
	aabb box_left;
	aabb box_right;
	hitable *ah = *(hitable **)a;
	hitable *bh = *(hitable **)b;

	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
	{
		std::cerr << "No bounding box in bvh_node constructor \n";
	}

	if (box_left.min().z() - box_right.min().z() < 0.0)
	{
		return - 1;
	}
	else
		return 1;
}

bvh_node::bvh_node(hitable **list, int n, float time0, float time1)
{
	// Choose a random axis on each recursive call to use to split the list.
	int axis = int(3 * drand48());

	if (axis == 0)
	{
		qsort(list, n, sizeof(hitable *), box_x_compare);
	}
	else if (axis == 1)
	{
		qsort(list, n, sizeof(hitable *), box_y_compare);
	}
	else
	{
		qsort(list, n, sizeof(hitable *), box_z_compare);
	}

	if (n == 1)
	{
		left = right  = list[0];
	}
	else if (n == 2)
	{
		left = list[0];
		right = list[1];
	}
	else
	{
		left = new bvh_node(list, n/2, time0, time1);
		right = new bvh_node(list + n/2, n - n/2, time0, time1);
	}

	aabb box_left;
	aabb box_right;

	if (!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right))
	{
		std::cerr << "No bounding box in bvh_node constructor \n";
	}

	box = sorrounding_box(box_left, box_right);
}

#endif // BVHHPP
