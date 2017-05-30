/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Debug/SpriteInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.SpriteInstanceDebugInfo", 0, SpriteInstanceDebugInfo, InstanceDebugInfo)

SpriteInstanceDebugInfo::SpriteInstanceDebugInfo()
:	m_mask(false)
,	m_clipped(false)
,	m_frames(0)
,	m_currentFrame(0)
,	m_playing(false)
{
}

SpriteInstanceDebugInfo::SpriteInstanceDebugInfo(const SpriteInstance* instance, const std::string& className, bool mask, bool clipped, const RefArray< InstanceDebugInfo >& childrenDebugInfo)
{
	m_name = instance->getName();
	m_bounds = instance->getVisibleLocalBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
	m_className = className;
	m_mask = mask;
	m_clipped = clipped;
	m_frames = instance->getSprite()->getFrameCount();
	m_currentFrame = instance->getCurrentFrame();
	m_playing = instance->getPlaying();
	m_scalingGrid = instance->getSprite()->getScalingGrid();
	m_childrenDebugInfo = childrenDebugInfo;
}

void SpriteInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> Member< std::string >(L"className", m_className);
	s >> Member< bool >(L"mask", m_mask);
	s >> Member< bool >(L"clipped", m_clipped);
	s >> Member< uint16_t >(L"frames", m_frames);
	s >> Member< uint16_t >(L"currentFrame", m_currentFrame);
	s >> Member< bool >(L"playing", m_playing);
	s >> MemberAabb2(L"scalingGrid", m_scalingGrid);
	s >> MemberRefArray< InstanceDebugInfo >(L"childrenDebugInfo", m_childrenDebugInfo);
}

	}
}
