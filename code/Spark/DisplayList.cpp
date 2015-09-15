#include "Spark/DisplayList.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.DisplayList", DisplayList, Object)

void DisplayList::place(int32_t depth, CharacterInstance* instance)
{
	m_layers[depth].instance = instance;
}

void DisplayList::remove(int32_t depth)
{
	SmallMap< int32_t, Layer >::iterator i = m_layers.find(depth);
	if (i != m_layers.end())
		m_layers.erase(i);
}

const SmallMap< int32_t, DisplayList::Layer >& DisplayList::getLayers() const
{
	return m_layers;
}

void DisplayList::getCharacters(RefArray< CharacterInstance >& outCharacters) const
{
	for (SmallMap< int32_t, Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		outCharacters.push_back(i->second.instance);
}

	}
}
