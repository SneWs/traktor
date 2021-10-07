#include "Spark/ShapeInstance.h"
#include "Spark/Shape.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ShapeInstance", ShapeInstance, CharacterInstance)

ShapeInstance::ShapeInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Shape* shape)
:	CharacterInstance(context, dictionary, parent)
,	m_shape(shape)
{
}

const Shape* ShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 ShapeInstance::getBounds() const
{
	return getTransform() * m_shape->getShapeBounds();
}

	}
}
