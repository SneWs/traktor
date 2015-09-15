#ifndef traktor_spark_Sprite_H
#define traktor_spark_Sprite_H

#include <map>
#include "Core/RefArray.h"
#include "Resource/Id.h"
#include "Spark/Character.h"

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

class Shape;

/*! \brief Sprite character.
 * \ingroup Spark
 */
class T_DLLCLASS Sprite : public Character
{
	T_RTTI_CLASS;

public:
	const Character* getCharacter(const std::wstring& id) const;

	virtual Ref< CharacterInstance > createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	friend class CharacterPipeline;

	resource::Id< Shape > m_shape;
	std::map< std::wstring, Ref< Character > > m_characters;
};

	}
}

#endif	// traktor_spark_Sprite_H
