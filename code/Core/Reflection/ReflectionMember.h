#ifndef traktor_ReflectionMember_H
#define traktor_ReflectionMember_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class T_DLLCLASS ReflectionMember : public Object
{
	T_RTTI_CLASS;

public:
	const wchar_t* getName() const;

protected:
	ReflectionMember(const wchar_t* name);

private:
	const wchar_t* m_name;
};

}

#endif	// traktor_ReflectionMember_H
