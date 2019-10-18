#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! Application active events.
 * \ingroup Runtime
 *
 * Applications are notified with this
 * event when Runtime is becoming activated or deactivated.
 */
class T_DLLCLASS ActiveEvent : public Object
{
	T_RTTI_CLASS;

public:
	explicit ActiveEvent(bool activated);

	bool becameActivated() const;

private:
	bool m_activated;
};

	}
}

