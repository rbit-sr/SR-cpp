#ifndef QUAD_TREE_NODE_H
#define QUAD_TREE_NODE_H

#include <cstdint>
#include <memory>
#include <vector>
#include <array>

#include "vector.h"
#include "aabb.h"
#include "interfaces.h"

namespace emu
{
	struct quad_tree_node : public i_clonable
	{
		vector m_center;
		int32_t m_depth;
		aabb m_bounds;
		std::array<std::unique_ptr<quad_tree_node>, 4> m_children;
		std::vector<i_quad_tree_leaf*> m_leaves;

		quad_tree_node(const aabb& bounds);
		quad_tree_node(const aabb& bounds, int32_t depth);
		quad_tree_node(const quad_tree_node& right);
		~quad_tree_node() = default;

		quad_tree_node& operator=(const quad_tree_node& right);
		quad_tree_node& operator=(quad_tree_node&& right) noexcept = default;

		std::unique_ptr<i_clonable> clone() const override;
		void replace_pointers(const std::map<const i_quad_tree_leaf*, i_quad_tree_leaf*>& map);

		void clear();
		void add(i_quad_tree_leaf* leaf);
		void remove(i_quad_tree_leaf* leaf);
		void update(i_quad_tree_leaf* leaf);
		void refresh_children();
		bool overlapping_center_axes(const aabb& bounds) const;
		quad_tree_node* get_overlapping_child(const aabb& bounds);
		void query_leaves(const aabb& query, std::vector<i_quad_tree_leaf*>& result);
		void query_leaves_recursion(const aabb& query, std::vector<i_quad_tree_leaf*>& result);
	};
}

#endif
