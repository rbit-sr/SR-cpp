#ifndef CONVEX_POLYGON_H
#define CONVEX_POLYGON_H

#include "interfaces.h"

namespace emu
{
	struct convex_polygon : public i_collision_shape
	{
		vector m_position;
		vector m_pivot;
		float m_rotation;
		std::vector<vector> m_local_vertices;
		std::vector<vector> m_vertices;
		vector m_unknown1;

		convex_polygon() = default;
		convex_polygon(const std::vector<vector>& local_vertices);
		convex_polygon(vector position, vector pivot, float rotation, const std::vector<vector>& local_vertices);
		convex_polygon(vector position, vector pivot, float rotation, std::vector<vector>&& local_vertices);
		convex_polygon(i_collision_shape* shape);
		~convex_polygon() = default;

		static convex_polygon create_obb(vector position, vector size, vector pivot, float rotation);
		shape get_id() const override;
		std::unique_ptr<i_clonable> clone() const override;
		void initialize();
		void calculate_vertices();
		int32_t get_vertex_count() const override;
		vector get_vertex(int32_t index) const override;
		void get_vertices(std::vector<vector>& vertices) const override;
		vector get_position() const override;
		void set_position(vector position) override;
		vector get_center() const override;
		vector get_pivot() const override;
		void set_pivot(vector pivot) override;
		float get_rotation() const override;
		void set_rotation(float rotation) override;
		void set_position_rotation(vector pos, float rotation);
	};
}

#endif
