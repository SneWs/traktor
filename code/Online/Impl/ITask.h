#ifndef traktor_online_ITask_H
#define traktor_online_ITask_H

#include "Core/Object.h"

namespace traktor
{
	namespace online
	{

class TaskQueue;

class ITask : public Object
{
	T_RTTI_CLASS;

public:
	virtual void execute(TaskQueue* taskQueue) = 0;
};

	}
}

#endif	// traktor_online_ITask_H
