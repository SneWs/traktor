#pragma once

#include "Scene/Editor/IEntityEditorFactory.h"

namespace traktor
{
	namespace physics
	{

/*! \brief
 */
class PhysicsEntityEditorFactory : public scene::IEntityEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityDataTypes() const override final;

	virtual Ref< scene::IEntityEditor > createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const override final;
};

	}
}

