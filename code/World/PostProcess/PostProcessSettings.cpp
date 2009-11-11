#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessDefine.h"
#include "World/PostProcess/PostProcessStep.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.world.PostProcessSettings", PostProcessSettings, Serializable)

PostProcessSettings::PostProcessSettings()
:	m_requireHighRange(false)
{
}

bool PostProcessSettings::requireHighRange() const
{
	return m_requireHighRange;
}

const RefArray< PostProcessDefine >& PostProcessSettings::getDefinitions() const
{
	return m_definitions;
}

const RefArray< PostProcessStep >& PostProcessSettings::getSteps() const
{
	return m_steps;
}

int PostProcessSettings::getVersion() const
{
	return 1;
}

bool PostProcessSettings::serialize(Serializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"requireHighRange", m_requireHighRange);

	s >> MemberRefArray< PostProcessDefine >(L"definitions", m_definitions);
	s >> MemberRefArray< PostProcessStep >(L"steps", m_steps);

	return true;
}

	}
}
