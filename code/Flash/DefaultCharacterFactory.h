#ifndef traktor_flash_DefaultCharacterFactory_H
#define traktor_flash_DefaultCharacterFactory_H

#include "Flash/ICharacterFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Matrix33;

	namespace flash
	{

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS DefaultCharacterFactory : public ICharacterFactory
{
	T_RTTI_CLASS;

public:
	virtual Ref< CharacterInstance > createInstance(
		const Character* character,
		int32_t depth,
		ActionContext* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const override final;

	virtual void removeInstance(CharacterInstance* instance, int32_t depth) const override final;
};

	}
}

#endif	// traktor_flash_DefaultCharacterFactory_H

