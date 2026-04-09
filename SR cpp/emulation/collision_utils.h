#ifndef COLLISION_UTILS_H
#define COLLISION_UTILS_H

#include <functional>

#include "interfaces.h"

namespace emu
{
	struct collision_utils
	{
		static bool intersect(i_collision_shape* source, i_collision_shape* target, vector& mtd);
		static bool intersect(vector p0, vector p1, vector q0, vector q1, vector& intersect_point);
		static float cross(vector a1, vector a2);
		static bool aabb_aabb_intersect(aabb* source, aabb* target, vector& mtd);
		static bool polygon_polygon_intersect(i_collision_shape* source, i_collision_shape* target, vector& mtd);
		static void unknown3(vector a0, const std::vector<vector>& a1, int32_t a2, float& a3, float& a4);
		static bool unknown4(float a0, float a1, float a2, float a3, float& a4);
		static void unknown5(vector a0, float a1, vector& a2, float& a3);

		static void assign_collision_shape(std::unique_ptr<i_collision_shape>& left, const i_collision_shape* right);
	};
}

#endif
