#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/ISoundResource.h"
#include "Sound/Editor/SoundAsset.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundAsset", 0, SoundAsset, editor::Asset)

SoundAsset::SoundAsset()
:	m_stream(false)
{
}

const TypeInfo* SoundAsset::getOutputType() const
{
	return &type_of< ISoundResource >();
}

bool SoundAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> Member< bool >(L"stream", m_stream);

	return true;
}

	}
}
