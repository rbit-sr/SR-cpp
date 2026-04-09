#include <cmath>

#include "convex_polygon.h"

using namespace emu;

convex_polygon::convex_polygon(const std::vector<vector>& local_vertices) :
	m_position{ vec_zero }, m_pivot{ vec_zero }, m_rotation{ 0.0f }
{
	m_local_vertices = local_vertices;
	initialize();
}

convex_polygon::convex_polygon(vector position, vector pivot, float rotation, const std::vector<vector>& local_vertices) :
	m_position{ position },
	m_pivot{ pivot },
	m_rotation{ rotation },
	m_local_vertices{ local_vertices }
{
	initialize();
}

convex_polygon::convex_polygon(vector position, vector pivot, float rotation, std::vector<vector>&& local_vertices) :
	m_position{ position },
	m_pivot{ pivot },
	m_rotation{ rotation },
	m_local_vertices{ std::move(local_vertices) }
{
	initialize();
}

convex_polygon::convex_polygon(i_collision_shape* shape)
{
	m_vertices.resize(shape->get_vertex_count());
	shape->get_vertices(m_vertices);
	m_position = shape->get_position();
	m_pivot = shape->get_pivot();
	m_rotation = shape->get_rotation();
	m_local_vertices.reserve(m_vertices.size());
	for (int32_t i = 0; i < m_vertices.size(); i++)
		m_local_vertices.push_back(m_vertices[i] - m_position);
	calculate_vertices();
}

convex_polygon convex_polygon::create_obb(vector position, vector size, vector pivot, float rotation)
{
	std::vector<vector> array = {
		vector{ 0.0f, 0.0f },
		vector{ size.x, 0.0f },
		vector{ size.x, size.y },
		vector{ 0.0f, size.y }
	};
	return convex_polygon(position, pivot, rotation, std::move(array));
}

shape convex_polygon::get_id() const
{
	return shape_convex_polygon;
}

std::unique_ptr<i_clonable> convex_polygon::clone() const
{
	return std::make_unique<convex_polygon>(m_position, m_pivot, m_rotation, m_local_vertices);
}

void convex_polygon::initialize()
{
	m_vertices.clear();
	m_vertices.resize(m_local_vertices.size());
	calculate_vertices();
}

void convex_polygon::calculate_vertices()
{
	m_unknown1 = vec_zero;
	float cos = 1.0f;
	float sin = 0.0f;

	if (m_rotation != 0.0f)
	{
		cos = std::cos(m_rotation);
		sin = std::sin(m_rotation);
	}

	if (cos == 1.0f)
	{
		for (int32_t i = 0; i < m_local_vertices.size(); i++)
		{
			vector vec = m_local_vertices[i];
			vec += m_position;
			m_vertices[i] = vec;
			m_unknown1 = m_unknown1 + vec;
		}
	}
	else
	{
		for (int32_t i = 0; i < m_local_vertices.size(); i++)
		{
			vector vec = m_local_vertices[i];
			float num = vec.x - m_pivot.x;
			float num2 = vec.y - m_pivot.y;
			vec.x = num * cos - num2 * sin;
			vec.y = num2 * cos + num * sin;
			vec += m_pivot + m_position;
			m_vertices[i] = vec;
			m_unknown1 = m_unknown1 + vec;
		}
	}
	m_unknown1 = m_unknown1 / (float)m_local_vertices.size();
}

int32_t convex_polygon::get_vertex_count() const
{
	return m_vertices.size();
}

vector convex_polygon::get_vertex(int32_t index) const
{
	return m_vertices[index];
}

void convex_polygon::get_vertices(std::vector<vector>& vertices) const
{
	if (vertices.size() < m_vertices.size())
		vertices.resize(m_vertices.size());

	for (size_t i = 0; i < m_vertices.size(); i++)
		vertices[i] = m_vertices[i];
}

vector convex_polygon::get_position() const
{
	return m_position;
}

void convex_polygon::set_position(vector position)
{
	m_position = position;
	calculate_vertices();
}

vector convex_polygon::get_center() const
{
	return m_unknown1;
}

vector convex_polygon::get_pivot() const
{
	return m_pivot;
}

void convex_polygon::set_pivot(vector pivot)
{
	m_pivot = pivot;
	calculate_vertices();
}

float convex_polygon::get_rotation() const
{
	return m_rotation;
}

void convex_polygon::set_rotation(float rotation)
{
	m_rotation = rotation;
	calculate_vertices();
}

void convex_polygon::set_position_rotation(vector pos, float rotation)
{
	m_position = pos;
	m_rotation = rotation;
	calculate_vertices();
}