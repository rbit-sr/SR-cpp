#ifndef STATE_H
#define STATE_H

#include <vector>
#include <array>

#include "common.h"
#include "actor.h"
#include "collision_engine.h"
#include "tile_layer_base.h"
#include "input.h"
#include "level.h"
#include "editable_actor.h"

namespace emu
{
	struct player;
	struct grapple;

	struct state
	{
		int32_t m_next_actor_id = 0;
		collision_engine m_collision_engine;
		timespan m_time = 0;

		std::array<std::array<bool, input_count>, 4> m_inputs{};

		state();
		state(level& level);
		state(const state& right);

		state& operator=(const state& right);

		std::vector<std::unique_ptr<actor>>& actors();
		const std::vector<std::unique_ptr<actor>>& actors() const;

		template <std::derived_from<i_actor_controller> T> requires
			std::is_default_constructible_v<T>
		T* spawn(vector position, int32_t id = -1)
		{
			std::unique_ptr<T> t = std::make_unique<T>();
			T* t_ptr = t.get();

			vector size;
			bool is_col = false;
			bool auto_col_det = false;
			bool should_pred_col = false;
			t_ptr->get_actor_params(size, is_col, auto_col_det, should_pred_col);

			std::unique_ptr<actor> actor = std::make_unique<emu::actor>(this, id == -1 ? m_next_actor_id++ : id, position, size);
			
			actor->d.is_collidable = is_col;
			actor->d.automatic_collision_detection = auto_col_det;
			actor->d.should_predict_collision = should_pred_col;

			actor->set_controller(std::move(t));
			t_ptr->reset();

			m_collision_engine.add_actor(std::move(actor));

			return t_ptr;
		}

		template <std::derived_from<editable_actor> T> requires
			std::is_constructible_v<T, level_actor>
		T* spawn(const level_actor& def, int32_t id = -1)
		{
			std::unique_ptr<T> t = std::make_unique<T>(def);
			T* t_ptr = t.get();

			vector dummy;
			bool is_col = false;
			bool auto_col_det = false;
			bool should_pred_col = false;
			t_ptr->get_actor_params(dummy, is_col, auto_col_det, should_pred_col);

			std::unique_ptr<actor> actor = std::make_unique<emu::actor>(this, id == -1 ? m_next_actor_id++ : id, def.position, def.size);

			actor->d.is_collidable = is_col;
			actor->d.automatic_collision_detection = auto_col_det;
			actor->d.should_predict_collision = should_pred_col;

			actor->set_controller(std::move(t));
			t_ptr->reset();

			m_collision_engine.add_actor(std::move(actor));

			return t_ptr;
		}

		template <std::derived_from<i_actor_controller> T>
		T* get_contr(size_t index)
		{
			for (auto& actor : actors())
			{
				if (T* contr = dynamic_cast<T*>(actor->m_controller.get()))
				{
					if (index == 0)
						return contr;
					index--;
				}
			}
			return nullptr;
		}

		template <std::derived_from<i_actor_controller> T>
		const T* get_contr(size_t index) const
		{
			for (const auto& actor : actors())
			{
				if (const T* contr = dynamic_cast<const T*>(actor->m_controller.get()))
				{
					if (index == 0)
						return contr;
					index--;
				}
			}
			return nullptr;
		}

		template <std::derived_from<i_actor_controller> T>
		size_t count() const
		{
			size_t count = 0;
			for (const auto& actor : actors())
			{
				if (const T* contr = dynamic_cast<const T*>(actor->m_controller.get()))
					count++;
			}
			return count;
		}

		void update(timespan delta);
	};
}

#endif
