#include "state.h"
#include "player.h"
#include "grapple.h"
#include "player_start.h"

using namespace emu;

state::state()
{
	
}

state::state(level& level)
{
	m_collision_engine.m_level = &level;

	player* player = spawn<emu::player>({ 0.0f, 0.0f });

	const level_actor& start_position = level.get_actor("PlayerStart");
	player->m_actor->set_position(start_position.position);
}

state::state(const state& right) :
	m_next_actor_id{ right.m_next_actor_id },
	m_collision_engine{ right.m_collision_engine },
	m_time{ right.m_time },
	m_inputs{ right.m_inputs }
{
	for (auto& actor : m_collision_engine.m_actors)
		actor->m_state = this;
}

state& state::operator=(const state& right)
{
	m_next_actor_id = right.m_next_actor_id;
	m_collision_engine = right.m_collision_engine;
	m_time = right.m_time;
	m_inputs = right.m_inputs;
	for (auto& actor : m_collision_engine.m_actors)
		actor->m_state = this;

	return *this;
}

std::vector<std::unique_ptr<actor>>& state::actors()
{
	return m_collision_engine.m_actors;
}

const std::vector<std::unique_ptr<actor>>& state::actors() const
{
	return m_collision_engine.m_actors;
}

void state::update(timespan delta)
{
	m_collision_engine.clear_collisions();
	for (auto& actor : m_collision_engine.m_actors)
		actor->update(m_time, delta);
	m_collision_engine.update_collisions(delta.seconds());
	for (auto& actor : m_collision_engine.m_actors)
	{
		actor->resolve_collision(m_time, delta);
	}
	m_time.ticks += delta;
}
