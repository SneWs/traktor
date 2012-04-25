#ifndef traktor_sound_SoundAsset_H
#define traktor_sound_SoundAsset_H

#include "Editor/Asset.h"

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

class T_DLLCLASS SoundAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	SoundAsset();

	virtual bool serialize(ISerializer& s);

private:
	friend class SoundPipeline;

	bool m_stream;
	bool m_preload;
};

	}
}

#endif	// traktor_sound_SoundAsset_H
