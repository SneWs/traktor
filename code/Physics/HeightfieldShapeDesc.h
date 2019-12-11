#pragma once

#include "Physics/ShapeDesc.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class Heightfield;
class MaterialMask;

	}

	namespace physics
	{

/*! Heightfield collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS HeightfieldShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	void setHeightfield(const resource::Id< hf::Heightfield >& heightfield);

	const resource::Id< hf::Heightfield >& getHeightfield() const;

	void setMaterialMask(const resource::Id< hf::MaterialMask >& materialMask);

	const resource::Id< hf::MaterialMask >& getMaterialMask() const;

	virtual void serialize(ISerializer& s) override final;

private:
	resource::Id< hf::Heightfield > m_heightfield;
	resource::Id< hf::MaterialMask > m_materialMask;
};

	}
}

