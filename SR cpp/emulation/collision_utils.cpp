#include <array>

#include "common.h"
#include "collision_utils.h"
#include "aabb.h"
#include "convex_polygon.h"
#include "line_trace.h"
#include "caches.h"
#include "config.h"

using namespace emu;

bool collision_utils::intersect(i_collision_shape* source, i_collision_shape* target, vector& mtd)
{
	shape source_shape = source->get_id();
	shape target_shape = target->get_id();

	if (source_shape == shape_aabb && target_shape == shape_aabb)
		return aabb_aabb_intersect(static_cast<aabb*>(source), static_cast<aabb*>(target), mtd);
#ifndef OPTIMIZE_COLLISION
	else if (source_shape == shape_convex_polygon && target_shape == shape_aabb)
		return polygon_polygon_intersect(static_cast<convex_polygon*>(source), static_cast<aabb*>(target), mtd);
#endif
	else if (source_shape == shape_line_trace && target_shape == shape_aabb)
		return polygon_polygon_intersect(static_cast<line_trace*>(source), static_cast<aabb*>(target), mtd);
	else if (source_shape == shape_aabb && target_shape == shape_convex_polygon)
		return polygon_polygon_intersect(static_cast<aabb*>(source), static_cast<convex_polygon*>(target), mtd);
#ifndef OPTIMIZE_COLLISION
	else if (source_shape == shape_convex_polygon && target_shape == shape_convex_polygon)
		return polygon_polygon_intersect(static_cast<convex_polygon*>(source), static_cast<convex_polygon*>(target), mtd);
#endif
	else if (source_shape == shape_line_trace && target_shape == shape_convex_polygon)
		return polygon_polygon_intersect(static_cast<line_trace*>(source), static_cast<convex_polygon*>(target), mtd);
#ifndef OPTIMIZE_COLLISION
	else if (source_shape == shape_aabb && target_shape == shape_line_trace)
		return polygon_polygon_intersect(static_cast<aabb*>(source), static_cast<line_trace*>(target), mtd);
	else if (source_shape == shape_convex_polygon && target_shape == shape_line_trace)
		return polygon_polygon_intersect(static_cast<convex_polygon*>(source), static_cast<line_trace*>(target), mtd);
	else if (source_shape == shape_line_trace && target_shape == shape_line_trace)
		return polygon_polygon_intersect(static_cast<line_trace*>(source), static_cast<line_trace*>(target), mtd);
#endif
	return false;
}

bool collision_utils::intersect(vector p0, vector p1, vector q0, vector q1, vector& intersect_point)
{
	vector vec = p1 - p0;
	vector vec2 = q1 - q0;
	float num = cross(vec, vec2);
	float num2 = cross(vec2, vec);
	if (num == 0.0f || num2 == 0.0f)
		return false;
	float num3 = cross(q0 - p0, vec2 / num);
	float num4 = cross(p0 - q0, vec / num2);
	if (num3 >= 0.0f && num3 <= 1.0f && num4 >= 0.0f && num4 <= 1.0f)
	{
		intersect_point = p0 + num3 * vec;
		return true;
	}
	return false;
}

float collision_utils::cross(vector a1, vector a2)
{
	return a1.x * a2.y - a1.y * a2.x;
}

bool collision_utils::aabb_aabb_intersect(aabb* source, aabb* target, vector& mtd)
{
	std::array<float, 4> deltas
	{
		target->min_x - source->max_x,
		target->max_x - source->min_x,
		target->min_y - source->max_y,
		target->max_y - source->min_y
	};

	int32_t num = 0;
	float num2 = std::abs(deltas[0]);
	for (int32_t i = 1; i <= 3; i++)
	{
		float num3 = std::abs(deltas[i]);
		if (num3 < num2)
		{
			num = i;
			num2 = num3;
		}
	}
	if (num <= 1)
	{
		mtd.x = deltas[num];
		mtd.y = 0.0f;
	}
	else
	{
		mtd.y = deltas[num];
		mtd.x = 0.0f;
	}
	return deltas[0] * deltas[1] <= 0.0f && deltas[2] * deltas[3] <= 0.0f;
}

bool collision_utils::do_projections_overlap(float start1, float end1, float start2, float end2, float& overlap_length)
{
	float d1 = end1 - start2;
	float d2 = end2 - start1;
	if (d1 < 0.0f || d2 < 0.0f)
		return false;
	overlap_length = d1 < d2 ? -d1 : d2;
	return true;
}

void collision_utils::update_mtd(vector axis, float overlap_length, vector& mtd, float& min)
{
	float axis_length_sqr = axis.length_sqr();
	float temp = overlap_length / axis_length_sqr;
	float td_length = temp * overlap_length;
	if (td_length < min)
	{
		min = td_length;
#ifdef OPTIMIZE_COLLISION
		mtd = axis * temp;
#else
		mtd = axis * overlap_length / axis_length_sqr;
#endif
	}
}

void collision_utils::assign_collision_shape(std::unique_ptr<i_collision_shape>& left, const i_collision_shape* right)
{
	if (right == nullptr)
	{
		left.reset();
		return;
	}

	if (left == nullptr)
	{
		left = ::clone<i_collision_shape>(right);
		return;
	}

	int32_t shape_left = left->get_id();
	int32_t shape_right = right->get_id();

	if (shape_left != shape_right)
		left = ::clone<i_collision_shape>(right);
	else
	{
		if (shape_left == shape_aabb)
			*static_cast<aabb*>(left.get()) = *static_cast<const aabb*>(right);
		else if (shape_left == shape_convex_polygon)
			*static_cast<convex_polygon*>(left.get()) = *static_cast<const convex_polygon*>(right);
		else if (shape_left == shape_line_trace)
			*static_cast<line_trace*>(left.get()) = *static_cast<const line_trace*>(right);
	}
}