#include "common.h"
#include "collision_engine.h"
#include "collision_utils.h"
#include "tile_layer_base.h"
#include "caches.h"

using namespace emu;

collision_engine::collision_engine()
{
	m_actors.reserve(128);
	m_auto_col_det_actors.reserve(128);
	m_add_collision_action = std::bind_front(&collision_engine::add_collision_pair, this);
}

collision_engine::collision_engine(const collision_engine& right) :
	m_quad_tree{ right.m_quad_tree },
	m_level{ right.m_level }
{
	// be careful when cloning the collision engine to correctly clone
	// all its members and pointers referencing them

	std::map<const i_quad_tree_leaf*, i_quad_tree_leaf*> leaf_map;
	std::map<const i_actor_controller*, i_actor_controller*> contr_map;

	m_actors.reserve(128);
	m_auto_col_det_actors.reserve(128);

	for (const auto& actor : right.m_actors)
	{
		::actor* newActor = m_actors.emplace_back(::clone<::actor>(actor.get())).get();
		leaf_map.emplace(actor.get(), newActor);
		contr_map.emplace(actor->m_controller.get(), newActor->m_controller.get());
		if (newActor->d.automatic_collision_detection)
			m_auto_col_det_actors.push_back(newActor);
	}

	m_quad_tree.replace_pointers(leaf_map);

	for (const auto& actor : m_actors)
		actor->m_controller->replace_pointers(contr_map);

	m_add_collision_action = std::bind_front(&collision_engine::add_collision_pair, this);
}

collision_engine& collision_engine::operator=(const collision_engine& right)
{
	// be careful when cloning the collision engine to correctly clone
	// all its members and pointers referencing them

	m_level = right.m_level;

	std::map<const i_quad_tree_leaf*, i_quad_tree_leaf*> leaf_map;
	std::map<const i_actor_controller*, i_actor_controller*> contr_map;

	m_auto_col_det_actors.clear();

	auto it = m_actors.begin();
	auto it_r = right.m_actors.begin();

	// reuse actors when possible
	for (; it != m_actors.end() && it_r != right.m_actors.end(); ++it, ++it_r)
	{
		*(*it) = *(*it_r);
		actor* actor = it->get();
		leaf_map.emplace(it_r->get(), actor);
		contr_map.emplace((*it_r)->m_controller.get(), actor->m_controller.get());
		if (actor->d.automatic_collision_detection)
			m_auto_col_det_actors.push_back(actor);
	}
	if (it == m_actors.end())
	{
		for (; it_r != right.m_actors.end(); ++it_r)
		{
			actor* newActor = m_actors.emplace_back(::clone<actor>(it_r->get())).get();
			leaf_map.emplace(it_r->get(), newActor);
			contr_map.emplace((*it_r)->m_controller.get(), newActor->m_controller.get());
			if (newActor->d.automatic_collision_detection)
				m_auto_col_det_actors.push_back(newActor);
		}
	}
	else
		m_actors.erase(it, m_actors.end());

	m_quad_tree = right.m_quad_tree;
	m_quad_tree.replace_pointers(leaf_map);

	for (const auto& actor : m_actors)
		actor->m_controller->replace_pointers(contr_map);

	return *this;
}

void collision_engine::clear()
{
	m_quad_tree.clear();
	m_actors.clear();
	m_auto_col_det_actors.clear();
}

void collision_engine::reset_actors()
{
	for (auto& actor : m_actors)
		actor->get_controller()->reset();
}

void collision_engine::add_actor(std::unique_ptr<actor>&& actor)
{
	::actor* actor_ptr = actor.get();
	m_actors.emplace_back(std::move(actor));
	if (actor_ptr->d.automatic_collision_detection)
		m_auto_col_det_actors.push_back(actor_ptr);
	if (actor_ptr->d.is_collidable)
		m_quad_tree.add(actor_ptr);
}

void collision_engine::remove_actor(actor* actor)
{
	if (actor->d.is_collidable)
	{
		m_quad_tree.remove(actor);
	}
	if (actor->d.automatic_collision_detection)
	{
		auto it2 = std::ranges::find(m_auto_col_det_actors, actor);
		if (it2 != m_auto_col_det_actors.end())
			m_auto_col_det_actors.erase(it2);
	}

	auto it1 = std::ranges::find_if(m_actors, [actor](const auto& u_ptr)
		{
			return actor == u_ptr.get();
		});
	if (it1 != m_actors.end())
		m_actors.erase(it1);
}

void collision_engine::swap_actors(actor* actor1, actor* actor2)
{
	auto it1 = std::ranges::find_if(m_actors, [actor1](const auto& u_ptr)
		{
			return actor1 == u_ptr.get();
		});
	auto it2 = std::ranges::find_if(m_actors, [actor2](const auto& u_ptr)
		{
			return actor2 == u_ptr.get();
		});

	if (it1 != m_actors.end() && it2 != m_actors.end())
	{
		std::swap(*it1, *it2);
	}
}

int32_t collision_engine::get_actor_count() const
{
	return m_actors.size();
}

actor* collision_engine::get_actor(int32_t index)
{
	return m_actors[index].get();
}

actor* collision_engine::get_actor_by_id(int32_t id)
{
	auto it = std::ranges::find_if(m_actors, [id](const auto& actor)
		{
			return actor->d.id == id;
		});
	if (it != m_actors.end())
		return it->get();

	return nullptr;
}

void collision_engine::add_collision_pair(
	i_collidable* source, i_collidable* target,
	vector source_pos, vector source_vel,
	vector target_pos, vector target_vel)
{
	caches::inst.collision_pairs.emplace_back(source, target, source_pos, source_vel, target_pos, target_vel);
}

void collision_engine::unknown1(const aabb& a1)
{
	m_quad_tree = quad_tree_node{ a1 };
	for (auto& actor : m_actors)
	{
		if (actor != nullptr && actor->d.is_collidable)
			m_quad_tree.add(actor.get());
	}
}

int32_t collision_engine::trace(vector start_point, vector end_point, std::vector<collision_pair*>& collisions)
{
	return trace(start_point, end_point, filter_all, collisions);
}

int32_t collision_engine::trace(vector start_point, vector end_point, collision_filter filter, std::vector<collision_pair*>& collisions)
{
#ifdef OPTIMIZE_COLLISION
	if (start_point.y == end_point.y)
	{
		if (start_point.x > end_point.x)
			std::swap(start_point.x, end_point.x);
		if (!m_level->is_colliding_hori(start_point, end_point.x - start_point.x))
			return 0;
	}
	else if (start_point.x == end_point.x)
	{
		if (start_point.y > end_point.y)
			std::swap(start_point.y, end_point.y);
		if (!m_level->is_colliding_vert(start_point, end_point.y - start_point.y))
			return 0;
	}
#endif

	auto& trace_line_actors = caches::inst.trace_line_actors;
	auto& trace_line_actors_count = caches::inst.trace_line_actors_count;

	if (trace_line_actors.size() == trace_line_actors_count)
		trace_line_actors.resize(trace_line_actors_count + 1);

	auto& trace_line_actor = trace_line_actors[trace_line_actors_count++];

	if (trace_line_actor == nullptr)
		trace_line_actor = std::make_unique<::trace_line_actor>(start_point, end_point, filter);
	else
		trace_line_actor->set(start_point, end_point, filter);

	get_collision_candidates(trace_line_actor.get());
	return check_collision_candidates_from_buf(collisions);
}

collision_pair* collision_engine::trace_single(vector start_point, vector end_point)
{
	return trace_single(start_point, end_point, filter_all, 50.0f);
}

collision_pair* collision_engine::trace_single(vector start_point, vector end_point, collision_filter filter)
{
	return trace_single(start_point, end_point, filter, 50.0f);
}

collision_pair* collision_engine::trace_single(vector start_point, vector end_point, collision_filter filter, float segment_length)
{
	vector vec = start_point;
	vector vec2 = end_point - start_point;
	float num = vec2.length();
	vec2 = vec2 / num;
	while (num > 0.0f)
	{
		vector vec3 = vec;
		vec = vec3 + vec2 * std::min(num, segment_length);
		num -= segment_length;

		auto& trace_line_actors = caches::inst.trace_line_actors;
		auto& trace_line_actors_count = caches::inst.trace_line_actors_count;

		if (trace_line_actors.size() == trace_line_actors_count)
			trace_line_actors.resize(trace_line_actors_count + 1);

		auto& trace_line_actor = trace_line_actors[trace_line_actors_count++];

		if (trace_line_actor == nullptr)
			trace_line_actor = std::make_unique<::trace_line_actor>(vec3, vec, filter);
		else
			trace_line_actor->set(vec3, vec, filter);

		get_collision_candidates(trace_line_actor.get());
		int32_t num2 = check_collision_candidates_from_buf(caches::inst.collision_pairs_ptr);
		if (num2 > 0)
		{
			collision_pair* result = caches::inst.collision_pairs_ptr[0];
			float num3 = FLOAT_MAX;
			for (int32_t i = 0; i < num2; i++)
			{
				collision_pair* collision_pair = caches::inst.collision_pairs_ptr[i];
				float num4 = (collision_pair->m_target->get_collision()->get_center() - vec3).length_sqr();
				if (num4 < num3)
				{
					result = collision_pair;
					num3 = num4;
				}
			}
			return result;
		}
	}
	return nullptr;
}

int32_t collision_engine::check_shape(const aabb& bounds, i_collision_shape& collision, std::vector<collision_pair*>& collisions)
{
	return check_shape(bounds, collision, filter_all, collisions);
}

int32_t collision_engine::check_shape(
	const aabb& bounds, 
	i_collision_shape& collision, 
	collision_filter filter, 
	std::vector<collision_pair*>& collisions)
{
#ifdef OPTIMIZE_COLLISION
	if (!m_level->is_colliding(bounds))
		return 0;
#endif

	auto& trace_polygon_actors = caches::inst.trace_polygon_actors;
	auto& trace_polygon_actors_count = caches::inst.trace_polygon_actors_count;

	if (trace_polygon_actors.size() == trace_polygon_actors_count)
		trace_polygon_actors.resize(trace_polygon_actors_count + 1);

	auto& trace_polygon_actor = trace_polygon_actors[trace_polygon_actors_count++];

	if (trace_polygon_actor == nullptr)
		trace_polygon_actor = std::make_unique<::trace_polygon_actor>(bounds, &collision, filter);
	else
		trace_polygon_actor->set(bounds, &collision, filter);

	get_collision_candidates(trace_polygon_actor.get());
	return check_collision_candidates_from_buf(collisions);
}

void collision_engine::refresh_collisions_on_actor(actor* actor)
{
	actor->reset_collision(0.0f, false);
	actor->reset_changed();

#ifdef OPTIMIZE_COLLISION
	if (!m_level->is_colliding(actor->m_bounds))
		return;
#endif

	if (actor->d.is_collidable)
		m_quad_tree.update(actor);

	int32_t collisions_on_actor = get_collisions_on_actor(actor, caches::inst.collision_pairs_ptr);
	for (int32_t i = 0; i < collisions_on_actor; i++)
		actor->add_collision2(*caches::inst.collision_pairs_ptr[i]);
}

void collision_engine::refresh_collisions_on_actor(actor* actor, float remaining_delta_time)
{
	actor->reset_collision(remaining_delta_time, false);
	actor->reset_changed();
	if (actor->d.is_collidable)
		m_quad_tree.update(actor);

#ifdef OPTIMIZE_COLLISION
	if (!m_level->is_colliding(actor->m_bounds))
		return;
#endif

	get_collision_candidates(actor, actor->m_add_collision_action);
	check_collision_candidates(actor, remaining_delta_time);
}

int32_t collision_engine::get_collisions_on_actor(i_collidable* actor, std::vector<collision_pair*>& collisions)
{
	get_collision_candidates(actor);
	return check_collision_candidates_from_buf(collisions);
}

void collision_engine::clear_collisions()
{
	caches::inst.collision_pairs.clear();
	caches::inst.tile_actors_count = 0;
	caches::inst.trace_line_actors_count = 0;
	caches::inst.trace_polygon_actors_count = 0;
}

void collision_engine::update_collisions(float delta_s)
{
	for (auto& actor : m_actors)
	{
		if (actor != nullptr && actor->d.is_collidable)
		{
			actor->reset_collision(delta_s, true);
			actor->reset_changed();

#ifdef OPTIMIZE_COLLISION
			actor->d.colliding = m_level->is_colliding(actor->m_bounds);
			if (!actor->d.colliding)
				continue;
#endif

			if (actor->is_moving())
				m_quad_tree.update(actor.get());
		}
	}
	get_collision_candidates_all_actors();
	check_collision_candidates_all_actors(delta_s);
}

void collision_engine::get_collision_candidates_all_actors()
{
	for (actor* actor : m_auto_col_det_actors)
	{
		if (actor->get_collision() != nullptr && actor->d.is_collision_active)
		{
#ifdef OPTIMIZE_COLLISION
			if (!actor->d.colliding)
				continue;
#endif
			get_collision_candidates(actor, actor->m_add_collision_action);
		}
	}
}

void collision_engine::get_collision_candidates(i_collidable* collidable)
{
	if (collidable->get_collision() != nullptr)
	{
		caches::inst.collision_pairs.clear();
		get_collision_candidates(collidable, m_add_collision_action);
	}
}

void collision_engine::get_collision_candidates(
	i_collidable* collidable, 
	std::function<void(i_collidable*, i_collidable*, vector, vector, vector, vector)> add_collision)
{
	auto& query_results = caches::inst.query_results;

	query_results.clear();
	m_quad_tree.query_leaves(collidable->get_bounds(), query_results);
	for (int32_t i = 0; i < query_results.size(); i++)
	{
		i_collidable* query_result = query_results[i];
		if (
			query_result->get_collision() != nullptr && 
			collidable != query_result && 
			query_result->get_collision_filter().collides_with(collidable->get_collision_filter()))
		{
			add_collision(
				collidable, query_result, 
				collidable->get_position(), collidable->get_velocity(), 
				query_result->get_position(), query_result->get_velocity());
		}
	}

	auto& tile_actors = caches::inst.tile_actors;
	auto& tile_actors_count = caches::inst.tile_actors_count;
	
	int32_t prev_count = tile_actors_count;

	m_level->m_tile_layer.get_tile_actors_at(collidable->get_bounds(), collidable->get_collision_filter());
	for (int32_t j = prev_count; j < tile_actors_count; j++)
	{
		add_collision(
			collidable, tile_actors[j].get(),
			collidable->get_position(), collidable->get_velocity(),
			tile_actors[j]->get_position(), tile_actors[j]->get_velocity());
	}
}

void collision_engine::check_collision_candidates_all_actors(float a1)
{
	for (actor* actor : m_auto_col_det_actors)
	{
		if (actor->get_collision_count() > 0)
			check_collision_candidates(actor, a1);
	}
}

void collision_engine::check_collision_candidates(actor* actor, float delta_s)
{
	vector vec = vec_zero;
	vector vec2 = vec_zero;
	float collision_sweep_minimal_density = actor->d.collision_sweep_minimal_density;
	int32_t num;
	float num2;
	if (actor->d.should_predict_collision && actor->get_velocity().length_sqr() > 0.0f)
	{
		vec = actor->get_size();
		vec = vec.normalized();
		vec2.x = std::abs(actor->get_velocity().x);
		vec2.y = std::abs(actor->get_velocity().y);
		vec2 = vec2.normalized();
		if (vec2.x < vec.x)
		{
			num = (int32_t)std::ceil((double)(std::abs(actor->get_velocity().y * delta_s) / actor->get_size().y * collision_sweep_minimal_density));
		}
		else
		{
			num = (int32_t)std::ceil((double)(std::abs(actor->get_velocity().x * delta_s) / actor->get_size().x * collision_sweep_minimal_density));
		}
		num2 = delta_s / (float)num;
	}
	else
	{
		num = 0;
		num2 = delta_s;
	}
	// ignore error handling code
	actor->d.total_iterations = num + 1;
	actor->d.iteration_time_step = num2;
	int32_t collision_count = actor->get_collision_count();
	i_collision_shape* collision = actor->get_collision();
	vector pos = collision->get_position();
	for (int32_t i = 0; i <= num; i++)
	{
		collision->set_position(pos + actor->get_velocity() * (float)i * num2);
		for (int32_t j = 0; j < collision_count; j++)
		{
			collision_pair* collision2 = actor->get_collision(j);
			i_collidable* target = collision2->m_target;
			i_collision_shape* collision3 = target->get_collision();
			vector pos2 = collision3->get_position();
			collision3->set_position(pos2 + collision2->m_target_vel * (float)i * num2);
			if (i > 0)
			{
				actor->add_collision(actor, target, collision->get_position(), actor->get_velocity(), collision3->get_position(), target->get_velocity());
				collision2 = actor->get_collision(actor->get_collision_count() - 1);
			}
			collision2->m_iteration = i;
			collision2->m_is_colliding = collision_utils::intersect(collision, collision3, collision2->m_mtd);
			collision3->set_position(pos2);
		}
	}
	actor->get_collision()->set_position(pos);
}

int32_t collision_engine::check_collision_candidates_from_buf(std::vector<collision_pair*>& a1)
{
	auto& collision_pairs = caches::inst.collision_pairs;

	if (a1.size() < collision_pairs.size())
		a1.resize(collision_pairs.size());
	int32_t result = 0;
	for (int32_t i = 0; i < collision_pairs.size(); i++)
	{
		collision_pair& collision_pair = collision_pairs[i];
		i_collision_shape* collision = collision_pair.m_source->get_collision();
		i_collision_shape* collision2 = collision_pair.m_target->get_collision();
		collision_pair.m_is_colliding = collision_utils::intersect(collision, collision2, collision_pair.m_mtd);
		if (collision_pair.m_is_colliding && collision_pair.m_mtd.length_sqr() >= 0.0f)
			a1[result++] = &collision_pair;
		// ignore error handling code
	}
	return result;
}