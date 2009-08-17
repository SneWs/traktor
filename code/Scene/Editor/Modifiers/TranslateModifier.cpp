#include "Scene/Editor/Modifiers/TranslateModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.TranslateModifier", TranslateModifier, IModifier)

void TranslateModifier::draw(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Matrix44& worldTransform,
	render::PrimitiveRenderer* primitiveRenderer,
	int button
)
{
	const Scalar c_guideScale(0.1f);
	const Scalar c_guideMinLength(1.0f);

	Scalar cameraDistance = (viewTransform * worldTransform).translation().length();
	Scalar guideLength = max(cameraDistance * c_guideScale, c_guideMinLength);
	Scalar arrowLength = guideLength * Scalar(1.0f / 8.0f);

	primitiveRenderer->pushView(viewTransform);
	primitiveRenderer->pushWorld(worldTransform);
	primitiveRenderer->pushDepthEnable(false);

	uint32_t axisEnable = context->getAxisEnable();

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(guideLength, 0.0f, 0.0f, 0.0f),
		3.0f,
		Color(255, 0, 0, (axisEnable & SceneEditorContext::AeX) ? 255 : 64)
	);
	primitiveRenderer->drawArrowHead(
		Vector4(guideLength, 0.0f, 0.0f, 0.0f),
		Vector4(guideLength + arrowLength, 0.0f, 0.0f, 0.0f),
		0.5f,
		Color(255, 0, 0, 255)
	);

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, guideLength, 0.0f, 0.0f),
		3.0f,
		Color(0, 255, 0, (axisEnable & SceneEditorContext::AeY) ? 255 : 64)
	);
	primitiveRenderer->drawArrowHead(
		Vector4(0.0f, guideLength, 0.0f, 0.0f),
		Vector4(0.0f, guideLength + arrowLength, 0.0f, 0.0f),
		0.5f,
		Color(0, 255, 0, 255)
	);

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, guideLength, 0.0f),
		3.0f,
		Color(0, 0, 255, (axisEnable & SceneEditorContext::AeZ) ? 255 : 64)
	);
	primitiveRenderer->drawArrowHead(
		Vector4(0.0f, 0.0f, guideLength, 0.0f),
		Vector4(0.0f, 0.0f, guideLength + arrowLength, 0.0f),
		0.5f,
		Color(0, 0, 255, 255)
	);

	primitiveRenderer->popDepthEnable();
	primitiveRenderer->popWorld();
	primitiveRenderer->popView();
}

void TranslateModifier::adjust(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	const Vector4& worldDelta,
	int button,
	Matrix44& outTransform
)
{
	outTransform = outTransform * translate(worldDelta);
}

	}
}
