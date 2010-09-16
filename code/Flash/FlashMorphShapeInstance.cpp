#include "Flash/FlashMorphShapeInstance.h"
#include "Flash/FlashMorphShape.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMorphShapeInstance", FlashMorphShapeInstance, FlashCharacterInstance)

FlashMorphShapeInstance::FlashMorphShapeInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashMorphShape* shape)
:	FlashCharacterInstance(context, L"Object", parent)
,	m_shape(shape)
{
}

const FlashMorphShape* FlashMorphShapeInstance::getShape() const
{
	return m_shape;
}

SwfRect FlashMorphShapeInstance::getBounds() const
{
	SwfRect bounds = m_shape->getShapeBounds();
	bounds.min = getTransform() * bounds.min;
	bounds.max = getTransform() * bounds.max;
	return bounds;
}

	}
}
