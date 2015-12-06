#ifndef traktor_amalgam_TargetBuildEvent_H
#define traktor_amalgam_TargetBuildEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

class TargetBuildEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetBuildEvent(ui::EventSubject* sender, TargetInstance* instance);

	TargetInstance* getInstance() const;

private:
	Ref< TargetInstance > m_instance;
};

	}
}

#endif	// traktor_amalgam_TargetBuildEvent_H
