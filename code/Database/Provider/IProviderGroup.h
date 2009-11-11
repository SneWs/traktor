#ifndef traktor_db_IProviderGroup_H
#define traktor_db_IProviderGroup_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class IProviderInstance;

/*! \brief Provider group interface.
 * \ingroup Database
 */
class T_DLLCLASS IProviderGroup : public Object
{
	T_RTTI_CLASS(IProviderGroup)

public:
	virtual std::wstring getName() const = 0;

	virtual bool rename(const std::wstring& name) = 0;

	virtual bool remove() = 0;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) = 0;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) = 0;

	virtual bool getChildGroups(RefArray< IProviderGroup >& outChildGroups) = 0;

	virtual bool getChildInstances(RefArray< IProviderInstance >& outChildInstances) = 0;
};

	}
}

#endif	// traktor_db_IProviderGroup_H
