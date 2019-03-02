#pragma once

#include "Physics/ShapeDesc.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Box collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS BoxShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	BoxShapeDesc();

	void setExtent(const Vector4& extent);

	const Vector4& getExtent() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_extent;
};

	}
}

