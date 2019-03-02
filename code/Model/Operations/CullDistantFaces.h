#pragma once

#include "Core/Math/Aabb3.h"
#include "Model/IModelOperation.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief
 * \ingroup Model
 */
class T_DLLCLASS CullDistantFaces : public IModelOperation
{
	T_RTTI_CLASS;

public:
	CullDistantFaces(const Aabb3& viewerRegion);

	virtual bool apply(Model& model) const override final;

private:
	Aabb3 m_viewerRegion;
};

	}
}

