#ifndef EVENT_H
#define EVENT_H

#include "../emulation/timespan.h"
#include "../emulation/vector.h"
#include "../emulation/state.h"

namespace util
{
	enum event_type
	{
		evt_none,
		evt_land_ground,
		evt_walk_to_slope,
		evt_walk_to_ground,
		evt_walk_off_ground,
		evt_walk_off_slope,
		evt_hit_wall,
		evt_hit_ceil,
		evt_attach_wall,
		evt_slide_off_wall,
		evt_land_slope_floor,
		evt_hit_slope_ceil,
		evt_swing_stop
	};

	struct event
	{
		event_type event = evt_none;
		emu::timespan time = SIZE_MAX;
		emu::vector vel_bef = emu::vec_zero;
		emu::vector vel_aft = emu::vec_zero;

		static std::string_view to_string(event_type event);
	};

	struct get_event_helper
	{
		emu::timespan m_init_time;
		bool m_was_on_ground;
		emu::collidable_type m_init_ground_col_id;
		bool m_was_on_wall;
		bool m_was_swinging;
		emu::vector m_prev_vel;

		get_event_helper() = default;
		get_event_helper(const emu::state& state);

		event get_event(const emu::state& next_state);
	};
}

#endif
