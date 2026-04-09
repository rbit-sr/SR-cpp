#include "editable_actor.h"

using namespace emu;

std::unique_ptr<i_clonable> editable_actor::clone() const
{
	return std::make_unique<editable_actor>(*this);
}

bool editable_actor::set(const i_actor_controller* other)
{
	if (const editable_actor* ptr_cast = dynamic_cast<const editable_actor*>(other))
	{
		*this = *ptr_cast;
		return true;
	}
	return false;
}

void editable_actor::replace_pointers(const std::map<const i_actor_controller*, i_actor_controller*>& map)
{

}

void editable_actor::get_actor_params(vector& size, bool& is_col, bool& auto_col_det, bool& should_pred_col)
{

}

actor* editable_actor::get_actor()
{
	return m_actor;
}

void editable_actor::set_actor(actor* actor)
{
	m_actor = actor;
}

i_collision_shape* editable_actor::get_collision()
{
	return &m_bounds;
}

collision_filter& editable_actor::get_collision_filter()
{
	static collision_filter default_cf = collision_filter{ 0u, 0u };

	if (!de.unknown5)
		return de.collision_filter;
	return default_cf;
}

collidable_type editable_actor::get_collidable_type() const
{
	return col_invalid;
}

void editable_actor::resolve_collision(timespan time, timespan delta)
{

}

void editable_actor::init()
{
	if (de.is_editor)
	{
		de.color1 = 0xFFFFFFFFu;
		de.color2 = 0xFF00FFFFu;
	}
	if (de.unknown8)
		m_bounds = aabb{ m_actor->d.position.x, m_actor->d.position.x + m_actor->d.size.x, m_actor->d.position.y, m_actor->d.position.y + m_actor->d.size.y };
}

void editable_actor::reset()
{

}

void editable_actor::update(timespan time, timespan delta)
{

}
