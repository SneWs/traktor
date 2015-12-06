#ifndef traktor_amalgam_TargetBrowseEvent_H
#define traktor_amalgam_TargetBrowseEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

class TargetBrowseEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetBrowseEvent(ui::EventSubject* sender, TargetInstance* instance);

	TargetInstance* getInstance() const;

private:
	Ref< TargetInstance > m_instance;
};

	}
}

#endif	// traktor_amalgam_TargetBrowseEvent_H
