#include <algorithm>
#include <cmath>

#include "common.h"
#include "tile_layer_base.h"
#include "caches.h"

using namespace emu;

tile_layer_base::tile_layer_base() :
	m_width{ 0 }, m_height{ 0 }
{

}

tile_layer_base::tile_layer_base(int32_t width, int32_t height) :
	m_width{ width }, m_height{ height }, m_tilemap{ std::make_unique_for_overwrite<tile_id[]>(width * height) }
{
	std::fill(m_tilemap.get(), m_tilemap.get() + width * height, tile_air);
}

const tile_id* tile_layer_base::get_tilemap() const
{
	return m_tilemap.get();
}

tile_id tile_layer_base::get_tile(int32_t x, int32_t y) const
{
	if (m_tilemap == nullptr || x < 0 || x >= m_width || y < 0 || y >= m_height)
		return tile_air;

	return m_tilemap[static_cast<size_t>(x + m_width * y)];
}

void tile_layer_base::set_tile(int32_t x, int32_t y, tile_id tile)
{
	if (m_tilemap == nullptr)
		return;

	if (x >= 0 && x < m_width && y >= 0 && y < m_height)
	{
		m_tilemap[static_cast<size_t>(x + m_width * y)] = tile;
	}
}

int32_t tile_layer_base::get_width() const
{
	return m_width;
}

int32_t tile_layer_base::get_height() const
{
	return m_height;
}

void tile_layer_base::get_tile_actors_at(const aabb& aabb_query, collision_filter filter_query) const
{
	if (m_tilemap == nullptr)
		return;

	int32_t num = (int32_t)std::floor((double)(aabb_query.min_x / 16.0f));
	int32_t num2 = (int32_t)std::floor((double)(aabb_query.min_y / 16.0f));
	int32_t num3 = (int32_t)std::ceil((double)(aabb_query.max_x / 16.0f));
	int32_t num4 = (int32_t)std::ceil((double)(aabb_query.max_y / 16.0f));
	if (num == num3)
	{
		num--;
		num3++;
	}
	if (num2 == num4)
	{
		num2--;
		num4++;
	}
	num = std::clamp(num, 0, (int32_t)m_width);
	num2 = std::clamp(num2, 0, (int32_t)m_height);
	num3 = std::clamp(num3, 0, (int32_t)m_width);
	num4 = std::clamp(num4, 0, (int32_t)m_height);

	auto& tile_actors = caches::inst.tile_actors;
	auto& tile_actors_count = caches::inst.tile_actors_count;
	
	for (int32_t i = num; i < num3; i++)
	{
		for (int32_t j = num2; j < num4; j++)
		{
			int32_t index = get_tile(i, j);
			const std::unique_ptr<tile_actor>& tile_actor_template = all_tiles[index];
			if (tile_actor_template.get() != nullptr && tile_actor_template->get_collision_filter().collides_with(filter_query))
			{
				if (tile_actors.size() == tile_actors_count)
					tile_actors.resize(tile_actors_count + 1);

				auto& tile_actor = tile_actors[tile_actors_count++];

				if (tile_actor == nullptr)
					tile_actor = ::clone<emu::tile_actor>(tile_actor_template.get());
				else
					tile_actor->set(*tile_actor_template);

				tile_actor->set_position(vector{ (float)(i * 16), (float)(j * 16) });
			}
		}
	}
}
