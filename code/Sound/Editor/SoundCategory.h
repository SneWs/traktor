#ifndef traktor_sound_SoundCategory_H
#define traktor_sound_SoundCategory_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundCategory : public ISerializable
{
	T_RTTI_CLASS;

public:
	SoundCategory();

	virtual bool serialize(ISerializer& s);

	const Guid& getParent() const { return m_parent; }

	float getVolume() const { return m_volume; }

private:
	Guid m_parent;
	float m_volume;
};

	}
}

#endif	// traktor_sound_SoundCategory_H
