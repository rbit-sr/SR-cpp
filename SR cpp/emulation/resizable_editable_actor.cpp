#include "resizable_editable_actor.h"

using namespace emu;

std::unique_ptr<i_clonable> resizable_editable_actor::clone() const
{
	return std::make_unique<resizable_editable_actor>(*this);
}

bool resizable_editable_actor::set(const i_actor_controller* other)
{
	if (const resizable_editable_actor* ptr_cast = dynamic_cast<const resizable_editable_actor*>(other))
	{
		*this = *ptr_cast;
		return true;
	}
	return false;
}
