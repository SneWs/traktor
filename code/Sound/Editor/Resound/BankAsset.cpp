#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Sound.h"
#include "Sound/Resound/IGrainData.h"
#include "Sound/Editor/Resound/BankAsset.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.BankAsset", 0, BankAsset, ISerializable)

void BankAsset::addGrain(IGrainData* grain)
{
	m_grains.push_back(grain);
}

void BankAsset::removeGrain(IGrainData* grain)
{
	m_grains.remove(grain);
}

const RefArray< IGrainData >& BankAsset::getGrains() const
{
	return m_grains;
}

bool BankAsset::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IGrainData >(L"grains", m_grains);
}

	}
}
