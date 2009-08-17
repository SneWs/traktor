#ifndef traktor_scene_ScaleModifier_H
#define traktor_scene_ScaleModifier_H

#include "Scene/Editor/IModifier.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Scale modifier. */
class ScaleModifier : public IModifier
{
	T_RTTI_CLASS(ScaleModifier)

public:
	virtual void draw(
		SceneEditorContext* context,
		const Matrix44& viewTransform,
		const Matrix44& worldTransform,
		render::PrimitiveRenderer* primitiveRenderer,
		int button
	);

	virtual void adjust(
		SceneEditorContext* context,
		const Matrix44& viewTransform,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		const Vector4& worldDelta,
		int button,
		Matrix44& outTransform
	);
};

	}
}

#endif	// traktor_scene_ScaleModifier_H
