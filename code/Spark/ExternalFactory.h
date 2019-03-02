#pragma once

#include "Spark/ICharacterFactory.h"

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

/*! \brief External instance factory.
 * \ingroup Spark
 */
class T_DLLCLASS ExternalFactory : public ICharacterFactory
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getCharacterTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< Character > create(const Context* context, const ICharacterBuilder* builder, const CharacterData* characterData, const Character* parent, const std::wstring& name) const T_OVERRIDE T_FINAL;
};

	}
}

