#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Flash/FlashFrame.h"
#include "Flash/SwfMembers.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashFrame", 0, FlashFrame, ISerializable)

FlashFrame::FlashFrame()
:	m_backgroundColorChange(false)
{
	m_backgroundColor.red =
	m_backgroundColor.green =
	m_backgroundColor.blue =
	m_backgroundColor.alpha = 255;
}

void FlashFrame::setLabel(const std::string& label)
{
	m_label = label;
}

const std::string& FlashFrame::getLabel() const
{
	return m_label;
}

void FlashFrame::changeBackgroundColor(const SwfColor& backgroundColor)
{
	m_backgroundColor = backgroundColor;
	m_backgroundColorChange = true;
}

bool FlashFrame::hasBackgroundColorChanged() const
{
	return m_backgroundColorChange;
}

const SwfColor& FlashFrame::getBackgroundColor() const
{
	return m_backgroundColor;
}

void FlashFrame::placeObject(const PlaceObject& placeObject)
{
	T_ASSERT (placeObject.depth > 0);
	m_placeObjects[placeObject.depth] = placeObject;
}

void FlashFrame::removeObject(const RemoveObject& removeObject)
{
	T_ASSERT (removeObject.depth > 0);
	m_removeObjects[removeObject.depth] = removeObject;
}

void FlashFrame::addActionScript(const IActionVMImage* actionScript)
{
	m_actionScripts.push_back(actionScript);
}

const SmallMap< uint16_t, FlashFrame::PlaceObject >& FlashFrame::getPlaceObjects() const
{
	return m_placeObjects;
}

const SmallMap< uint16_t, FlashFrame::RemoveObject >& FlashFrame::getRemoveObjects() const
{
	return m_removeObjects;
}

const RefArray< const IActionVMImage >& FlashFrame::getActionScripts() const
{
	return m_actionScripts;
}

bool FlashFrame::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"label", m_label);
	s >> Member< bool >(L"backgroundColorChange", m_backgroundColorChange);
	
	if (m_backgroundColorChange)
		s >> MemberSwfColor(L"backgroundColor", m_backgroundColor);

	s >> MemberSmallMap< uint16_t, PlaceObject, Member< uint16_t >, MemberComposite< PlaceObject > >(L"placeObjects", m_placeObjects);
	s >> MemberSmallMap< uint16_t, RemoveObject, Member< uint16_t >, MemberComposite< RemoveObject > >(L"removeObjects", m_removeObjects);
	s >> MemberRefArray< const IActionVMImage >(L"actionScripts", m_actionScripts);

	return true;
}

bool FlashFrame::PlaceAction::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"eventMask", eventMask);
	s >> MemberRef< const IActionVMImage >(L"script", script);
	return true;
}

bool FlashFrame::PlaceObject::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"hasFlags", hasFlags);
	s >> Member< uint16_t >(L"depth", depth);

	if (hasBitmapCaching)
		s >> Member< uint8_t >(L"bitmapCaching", bitmapCaching);

	if (hasBlendMode)
		s >> Member< uint8_t >(L"blendMode", blendMode);

	if (hasActions)
		s >> MemberStlVector< PlaceAction, MemberComposite< PlaceAction > >(L"actions", actions);

	if (hasClipDepth)
		s >> Member< uint16_t >(L"clipDepth", clipDepth);

	if (hasName)
		s >> Member< std::string >(L"name", name);

	if (hasRatio)
		s >> Member< uint16_t >(L"ratio", ratio);

	if (hasCxTransform)
		s >> MemberSwfCxTransform(L"cxTransform", cxTransform);

	if (hasMatrix)
		s >> Member< Matrix33 >(L"matrix", matrix);

	if (hasCharacterId)
		s >> Member< uint16_t >(L"characterId", characterId);

	return true;
}

bool FlashFrame::RemoveObject::serialize(ISerializer& s)
{
	s >> Member< bool >(L"hasCharacterId", hasCharacterId);
	s >> Member< uint16_t >(L"depth", depth);

	if (hasCharacterId)
		s >> Member< uint16_t >(L"characterId", characterId);

	return true;
}

	}
}
