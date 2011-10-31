#include "Flash/FlashButton.h"
#include "Flash/FlashButtonInstance.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashButton", FlashButton, FlashCharacter)

FlashButton::FlashButton(uint16_t id)
:	FlashCharacter(id)
{
}

void FlashButton::addButtonLayer(const ButtonLayer& layer)
{
	m_layers.push_back(layer);
}

const FlashButton::button_layers_t& FlashButton::getButtonLayers() const
{
	return m_layers;
}

void FlashButton::addButtonCondition(const ButtonCondition& condition)
{
	m_conditions.push_back(condition);
}

const FlashButton::button_conditions_t& FlashButton::getButtonConditions() const
{
	return m_conditions;
}

Ref< FlashCharacterInstance > FlashButton::createInstance(ActionContext* context, FlashCharacterInstance* parent, const std::string& name, const ActionObject* initObject) const
{
	return new FlashButtonInstance(context, parent, this);
}

	}
}
