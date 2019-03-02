#pragma once

#include "Flash/CharacterInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class MorphShape;

/*! \brief Flash morph shape instance.
 * \ingroup Flash
 */
class T_DLLCLASS MorphShapeInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	MorphShapeInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const MorphShape* shape);

	const MorphShape* getShape() const;

	virtual Aabb2 getBounds() const override final;

private:
	Ref< const MorphShape > m_shape;
};

	}
}

