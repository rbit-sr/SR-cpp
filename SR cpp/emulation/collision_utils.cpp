#include "common.h"
#include "collision_utils.h"
#include "aabb.h"
#include "convex_polygon.h"
#include "line_trace.h"
#include "caches.h"

using namespace emu;

bool collision_utils::intersect(i_collision_shape* source, i_collision_shape* target, vector& mtd)
{
	if (source->get_id() == shape_aabb && target->get_id() == shape_aabb)
		return aabb_aabb_intersect(static_cast<aabb*>(source), static_cast<aabb*>(target), mtd);
	else
		return polygon_polygon_intersect(source, target, mtd);
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

bool collision_utils::polygon_polygon_intersect(i_collision_shape* source, i_collision_shape* target, vector& mtd)
{
	auto& vertices1 = caches::inst.vertices1;
	auto& vertices2 = caches::inst.vertices2;

	source->get_vertices(vertices1);
	target->get_vertices(vertices2);
	float max_val = FLOAT_MAX;
	float min_val = FLOAT_MIN;
	float max_val2 = FLOAT_MAX;
	float min_val2 = FLOAT_MIN;
	float num = 0.0f;
	float num2 = -1.0f;
	int32_t i = 0;
	int32_t num3 = source->get_vertex_count() - 1;
	while (i < source->get_vertex_count())
	{
		vector vec = vertices1[i] - vertices1[num3];
		vector vec2 = vector(-vec.y, vec.x);
		unknown3(vec2, vertices1, source->get_vertex_count(), max_val, min_val);
		unknown3(vec2, vertices2, target->get_vertex_count(), max_val2, min_val2);
		if (!unknown4(max_val, min_val, max_val2, min_val2, num))
			return false;
		unknown5(vec2, num, mtd, num2);
		num3 = i;
		i++;
	}
	int32_t j = 0;
	int32_t num4 = target->get_vertex_count() - 1;
	while (j < target->get_vertex_count())
	{
		vector vec3 = vertices2[j] - vertices2[num4];
		vector vec2 = vector(-vec3.y, vec3.x);
		unknown3(vec2, vertices1, source->get_vertex_count(), max_val, min_val);
		unknown3(vec2, vertices2, target->get_vertex_count(), max_val2, min_val2);
		if (!unknown4(max_val, min_val, max_val2, min_val2, num))
			return false;
		unknown5(vec2, num, mtd, num2);
		num4 = j;
		j++;
	}
	return true;
}

void collision_utils::unknown3(vector a0, const std::vector<vector>& a1, int32_t a2, float& a3, float& a4)
{
	a3 = (a4 = a1[0].dot(a0));
	for (int32_t i = 1; i < a2; i++)
	{
		float num = a1[i].dot(a0);
		if (num < a3)
			a3 = num;
		else if (num > a4)
			a4 = num;
	}
}

bool collision_utils::unknown4(float a0, float a1, float a2, float a3, float& a4)
{
	float num = a1 - a2;
	float num2 = a3 - a0;
	if (num < 0.0f || num2 < 0.0f)
		return false;
	a4 = ((num < num2) ? (-num) : num2);
	return true;
}

void collision_utils::unknown5(vector a0, float a1, vector& a2, float& a3)
{
	float num = a0.length_sqr();
	float num2 = a1 / num * a1;
	if (a3 < 0.0f || num2 < a3)
	{
		a3 = num2;
		a2 = a0 * a1 / num;
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