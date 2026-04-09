#include "player_start.h"

using namespace emu;

player_start::player_start()
{
	de.unknown4 = true;
	de.collision_filter = collision_filter{ 0x1, 0x0 };
}

std::unique_ptr<i_clonable> player_start::clone() const
{
	return std::make_unique<player_start>(*this);
}

bool player_start::set(const i_actor_controller* other)
{
	if (const player_start* ptr_cast = dynamic_cast<const player_start*>(other))
	{
		*this = *ptr_cast;
		return true;
	}
	return false;
}

void player_start::replace_pointers(const std::map<const i_actor_controller*, i_actor_controller*>& map)
{

}

void player_start::get_actor_params(vector& size, bool& is_col, bool& auto_col_det, bool& should_pred_col)
{
	size = vector{ 25.0f, 45.0f };
	is_col = true;
	auto_col_det = false;
	should_pred_col = false;
}

collidable_type player_start::get_collidable_type() const
{
	return col_player_start;
}

void player_start::init()
{
	editable_actor::init();
}
