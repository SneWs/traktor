#pragma once

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Key;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS KeyMoveEvent : public Event
{
	T_RTTI_CLASS;

public:
	KeyMoveEvent(EventSubject* sender, Key* key, int32_t offset);

	Key* getKey() const;

	int32_t getOffset() const;

private:
	Ref< Key > m_key;
	int32_t m_offset;
};

	}
}

