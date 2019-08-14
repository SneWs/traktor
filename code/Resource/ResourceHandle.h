#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! Resource handle base class.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceHandle : public Object
{
	T_RTTI_CLASS;

public:
	/*! Replace resource object.
	 *
	 * \param object New resource object.
	 */
	void replace(Object* object) { m_object = object; }

	/*! Get resource object.
	 *
	 * \return Resource object.
	 */
	Object* get() const { return m_object; }

	/*! Flush resource object.
	 */
	void flush() { m_object = nullptr; }

protected:
	mutable Ref< Object > m_object;
};

	}
}

