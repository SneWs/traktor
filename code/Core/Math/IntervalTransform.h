#ifndef traktor_IntervalTransform_H
#define traktor_IntervalTransform_H

#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_MATH_ALIGN16 T_DLLCLASS IntervalTransform
{
public:
	IntervalTransform();

	IntervalTransform(const Transform& transform);

	void set(const Transform& transform);

	Transform get(float interval) const;

	void step();

	const Transform& get() const { return m_transform[1]; }

private:
	Transform m_transform[2];
	bool m_step;
};

}

#endif	// traktor_IntervalTransform_H
