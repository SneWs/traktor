#pragma once

#include "Core/Containers/SmallMap.h"
#include "Spark/ICharacterBuilder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class ICharacterFactory;

/*! \brief Default character builder.
 * \ingroup Spark
 */
class T_DLLCLASS CharacterBuilder : public ICharacterBuilder
{
	T_RTTI_CLASS;

public:
	void addFactory(ICharacterFactory* factory);

	virtual Ref< Character > create(const Context* context, const CharacterData* characterData, const Character* parent, const std::wstring& name) const T_OVERRIDE T_FINAL;

private:
	SmallMap< const TypeInfo*, Ref< ICharacterFactory > > m_factories;
};

	}
}

