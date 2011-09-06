#ifndef traktor_db_EvtInstanceRenamed_H
#define traktor_db_EvtInstanceRenamed_H

#include "Database/Events/EvtInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class T_DLLCLASS EvtInstanceRenamed : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceRenamed();

	EvtInstanceRenamed(const Guid& instanceGuid, const std::wstring& previousName);

	const std::wstring& getPreviousName() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_previousName;
};

	}
}

#endif	// traktor_db_EvtInstanceRenamed_H
