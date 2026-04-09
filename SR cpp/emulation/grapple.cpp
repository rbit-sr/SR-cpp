#include "grapple.h"
#include "common.h"
#include "tile_actor.h"
#include "player.h"

using namespace emu;

std::unique_ptr<i_clonable> grapple::clone() const
{
	return std::make_unique<grapple>(*this);
}

bool grapple::set(const i_actor_controller* other)
{
	if (const grapple* ptr_cast = dynamic_cast<const grapple*>(other))
	{
		*this = *ptr_cast;
		return true;
	}
	return false;
}

void grapple::replace_pointers(const std::map<const i_actor_controller*, i_actor_controller*>& map)
{
	auto it = map.find(m_owner);
	if (it != map.end())
		m_owner = dynamic_cast<player*>(it->second);
}

void grapple::get_actor_params(vector& size, bool& is_col, bool& auto_col_det, bool& should_pred_col)
{
	size = vector{ 12.0f, 12.0f };
	is_col = true;
	auto_col_det = true;
	should_pred_col = true;
}

actor* grapple::get_actor()
{
	return m_actor;
}

void grapple::set_actor(actor* actor)
{
	m_actor = actor;
}

i_collision_shape* grapple::get_collision()
{
	m_bounds = aabb(m_actor->d.position.x, m_actor->d.position.x + m_actor->d.size.x, m_actor->d.position.y, m_actor->d.position.y + m_actor->d.size.y);
	return &m_bounds;
}

collision_filter& grapple::get_collision_filter()
{
	return d.collision_filter;
}

collidable_type grapple::get_collidable_type() const
{
	return col_hook;
}

vector grapple::get_center() const
{
	return m_actor->d.position + m_actor->d.size / 2.0f;
}

void grapple::init()
{
	m_actor->d.is_collision_active = false;
}

void grapple::reset()
{

}

void grapple::connect(vector position)
{
	m_actor->d.is_collision_active = true;
	d.connected = true;
	// ignore graphics code
	m_actor->d.velocity = vec_zero;
	m_actor->set_position(position);
	// ignore sound code
}

void grapple::shoot(vector position, vector direction)
{
	m_actor->d.is_collision_active = true;
	d.connected = false;
	m_actor->set_position(position);
	m_actor->d.velocity = direction * 3000.0f;
	d.direction = direction;
	// ignore graphics code
	d.collision_filter.m_category_bits = 0x01000000u;
}

void grapple::cancel()
{
	m_actor->d.is_collision_active = false;
	d.connected = false;
	// ignore graphics code
	d.collision_filter.m_category_bits = 0x0u;
	m_actor->d.velocity = vec_zero;
}

void grapple::resolve_collision(timespan time, timespan delta)
{
	if (!m_actor->d.is_collision_active || d.connected)
		return;
	
	float delta_s = delta.seconds();
	vector cent = get_center();
	i_collidable* closest_tile_actor = nullptr;
	collision_pair* closest_collision = nullptr;
	float min_dist = FLOAT_MAX;
	bool flag = false;
	for (int32_t i = 0; i < m_actor->get_collision_count(); i++)
	{
		collision_pair* collision = m_actor->get_collision(i);
		if (collision->m_is_colliding)
		{
			flag = true;
			if (closest_collision == nullptr)
				closest_collision = collision;
			if (tile_actor* tile_actor = dynamic_cast<::tile_actor*>(collision->m_target))
			{
				float dist = (cent - tile_actor->get_bounds().get_center()).length_sqr();
				if (dist < min_dist || (dist == min_dist && tile_actor->get_collidable_type() == 6))
				{
					closest_collision = collision;
					closest_tile_actor = collision->m_target;
					min_dist = dist;
				}
			}
		}
	}
	if (flag)
	{
		if (closest_tile_actor != nullptr && closest_tile_actor->get_collidable_type() == 6)
		{
			connect(vector(
				std::min(closest_tile_actor->get_bounds().max_x - 1.5f * m_bounds.width, std::max(closest_tile_actor->get_bounds().min_x - 0.5f * m_bounds.width, m_actor->d.position.x)),
				closest_tile_actor->get_bounds().max_y
			));
			// ignore sound code
		}
		else
		{
			if (closest_collision != nullptr)
				m_actor->d.position = closest_collision->m_source_pos + closest_collision->m_mtd;
			// ignore graphics code
			cancel();
			// ignore sound code
		}
	}
	else
		m_actor->set_position(m_actor->d.position + m_actor->d.velocity * delta_s);

	// ignore graphics and sound code
}

void grapple::update(timespan time, timespan delta)
{
	// ignore graphics code
}
