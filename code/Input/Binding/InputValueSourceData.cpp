#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Input/Binding/InputValueSourceData.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const MemberEnum< InputCategory >::Key c_InputCategory_Keys[] =
{
	{ L"CtKeyboard", CtKeyboard },
	{ L"CtMouse", CtMouse },
	{ L"CtJoystick", CtJoystick },
	{ L"CtWheel", CtWheel },
	0
};

const MemberEnum< InputDefaultControlType >::Key c_InputDefaultControlType_Keys[] =
{
	{ L"DtInvalid", DtInvalid },
	{ L"DtUp", DtUp },
	{ L"DtDown", DtDown },
	{ L"DtLeft", DtLeft },
	{ L"DtRight", DtRight },
	{ L"DtSelect", DtSelect },
	{ L"DtCancel", DtCancel },
	{ L"DtThumbLeftX", DtThumbLeftX },
	{ L"DtThumbLeftY", DtThumbLeftY },
	{ L"DtThumbLeftPush", DtThumbLeftPush },
	{ L"DtThumbRightX", DtThumbRightX },
	{ L"DtThumbRightY", DtThumbRightY },
	{ L"DtThumbRightPush", DtThumbRightPush },
	{ L"DtTriggerLeft", DtTriggerLeft },
	{ L"DtTriggerRight", DtTriggerRight },
	{ L"DtShoulderLeft", DtShoulderLeft },
	{ L"DtShoulderRight", DtShoulderRight },
	{ L"DtButton1", DtButton1 },
	{ L"DtButton2", DtButton2 },
	{ L"DtButton3", DtButton3 },
	{ L"DtButton4", DtButton4 },
	{ L"DtAxisX", DtAxisX },
	{ L"DtAxisY", DtAxisY },
	{ L"DtKeyEscape", DtKeyEscape },
	{ L"DtKey1", DtKey1 },
	{ L"DtKey2", DtKey2 },
	{ L"DtKey3", DtKey3 },
	{ L"DtKey4", DtKey4 },
	{ L"DtKey5", DtKey5 },
	{ L"DtKey6", DtKey6 },
	{ L"DtKey7", DtKey7 },
	{ L"DtKey8", DtKey8 },
	{ L"DtKey9", DtKey9 },
	{ L"DtKey0", DtKey0 },
	{ L"DtKeyMinus", DtKeyMinus },
	{ L"DtKeyEquals", DtKeyEquals },
	{ L"DtKeyBack", DtKeyBack },
	{ L"DtKeyTab", DtKeyTab },
	{ L"DtKeyQ", DtKeyQ },
	{ L"DtKeyW", DtKeyW },
	{ L"DtKeyE", DtKeyE },
	{ L"DtKeyR", DtKeyR },
	{ L"DtKeyT", DtKeyT },
	{ L"DtKeyY", DtKeyY },
	{ L"DtKeyU", DtKeyU },
	{ L"DtKeyI", DtKeyI },
	{ L"DtKeyO", DtKeyO },
	{ L"DtKeyP", DtKeyP },
	{ L"DtKeyLeftBracket", DtKeyLeftBracket },
	{ L"DtKeyRightBracket", DtKeyRightBracket },
	{ L"DtKeyReturn", DtKeyReturn },
	{ L"DtKeyLeftControl", DtKeyLeftControl },
	{ L"DtKeyA", DtKeyA },
	{ L"DtKeyS", DtKeyS },
	{ L"DtKeyD", DtKeyD },
	{ L"DtKeyF", DtKeyF },
	{ L"DtKeyG", DtKeyG },
	{ L"DtKeyH", DtKeyH },
	{ L"DtKeyJ", DtKeyJ },
	{ L"DtKeyK", DtKeyK },
	{ L"DtKeyL", DtKeyL },
	{ L"DtKeySemicolon", DtKeySemicolon },
	{ L"DtKeyApostrophe", DtKeyApostrophe },
	{ L"DtKeyGrave", DtKeyGrave },
	{ L"DtKeyLeftShift", DtKeyLeftShift },
	{ L"DtKeyBackSlash", DtKeyBackSlash },
	{ L"DtKeyZ", DtKeyZ },
	{ L"DtKeyX", DtKeyX },
	{ L"DtKeyC", DtKeyC },
	{ L"DtKeyV", DtKeyV },
	{ L"DtKeyB", DtKeyB },
	{ L"DtKeyN", DtKeyN },
	{ L"DtKeyM", DtKeyM },
	{ L"DtKeyComma", DtKeyComma },
	{ L"DtKeyPeriod", DtKeyPeriod },
	{ L"DtKeySlash", DtKeySlash },
	{ L"DtKeyRightShift", DtKeyRightShift },
	{ L"DtKeyMultiply", DtKeyMultiply },
	{ L"DtKeyLeftMenu", DtKeyLeftMenu },
	{ L"DtKeySpace", DtKeySpace },
	{ L"DtKeyCaptial", DtKeyCaptial },
	{ L"DtKeyF1", DtKeyF1 },
	{ L"DtKeyF2", DtKeyF2 },
	{ L"DtKeyF3", DtKeyF3 },
	{ L"DtKeyF4", DtKeyF4 },
	{ L"DtKeyF5", DtKeyF5 },
	{ L"DtKeyF6", DtKeyF6 },
	{ L"DtKeyF7", DtKeyF7 },
	{ L"DtKeyF8", DtKeyF8 },
	{ L"DtKeyF9", DtKeyF9 },
	{ L"DtKeyF10", DtKeyF10 },
	{ L"DtKeyNumLock", DtKeyNumLock },
	{ L"DtKeyScroll", DtKeyScroll },
	{ L"DtKeyNumPad7", DtKeyNumPad7 },
	{ L"DtKeyNumPad8", DtKeyNumPad8 },
	{ L"DtKeyNumPad9", DtKeyNumPad9 },
	{ L"DtKeySubtract", DtKeySubtract },
	{ L"DtKeyNumPad4", DtKeyNumPad4 },
	{ L"DtKeyNumPad5", DtKeyNumPad5 },
	{ L"DtKeyNumPad6", DtKeyNumPad6 },
	{ L"DtKeyAdd", DtKeyAdd },
	{ L"DtKeyNumPad1", DtKeyNumPad1 },
	{ L"DtKeyNumPad2", DtKeyNumPad2 },
	{ L"DtKeyNumPad3", DtKeyNumPad3 },
	{ L"DtKeyNumPad0", DtKeyNumPad0 },
	{ L"DtKeyDecimal", DtKeyDecimal },
	{ L"DtKeyOem102", DtKeyOem102 },
	{ L"DtKeyF11", DtKeyF11 },
	{ L"DtKeyF12", DtKeyF12 },
	{ L"DtKeyF13", DtKeyF13 },
	{ L"DtKeyF14", DtKeyF14 },
	{ L"DtKeyF15", DtKeyF15 },
	{ L"DtKeyKana", DtKeyKana },
	{ L"DtKeyAbntC1", DtKeyAbntC1 },
	{ L"DtKeyConvert", DtKeyConvert },
	{ L"DtKeyNoConvert", DtKeyNoConvert },
	{ L"DtKeyYen", DtKeyYen },
	{ L"DtKeyAbntC2", DtKeyAbntC2 },
	{ L"DtKeyNumPadEquals", DtKeyNumPadEquals },
	{ L"DtKeyPreviousTrack", DtKeyPreviousTrack },
	{ L"DtKeyAt", DtKeyAt },
	{ L"DtKeyColon", DtKeyColon },
	{ L"DtKeyUnderline", DtKeyUnderline },
	{ L"DtKeyKanji", DtKeyKanji },
	{ L"DtKeyStop", DtKeyStop },
	{ L"DtKeyAx", DtKeyAx },
	{ L"DtKeyUnlabeled", DtKeyUnlabeled },
	{ L"DtKeyNextTrack", DtKeyNextTrack },
	{ L"DtKeyNumPadEnter", DtKeyNumPadEnter },
	{ L"DtKeyRightControl", DtKeyRightControl },
	{ L"DtKeyMute", DtKeyMute },
	{ L"DtKeyCalculator", DtKeyCalculator },
	{ L"DtKeyPlayPause", DtKeyPlayPause },
	{ L"DtKeyMediaStop", DtKeyMediaStop },
	{ L"DtKeyVolumeDown", DtKeyVolumeDown },
	{ L"DtKeyVolumeUp", DtKeyVolumeUp },
	{ L"DtKeyWebHome", DtKeyWebHome },
	{ L"DtKeyNumPadComma", DtKeyNumPadComma },
	{ L"DtKeyDivide", DtKeyDivide },
	{ L"DtKeySysRq", DtKeySysRq },
	{ L"DtKeyRightMenu", DtKeyRightMenu },
	{ L"DtKeyPause", DtKeyPause },
	{ L"DtKeyHome", DtKeyHome },
	{ L"DtKeyUp", DtKeyUp },
	{ L"DtKeyPrior", DtKeyPrior },
	{ L"DtKeyLeft", DtKeyLeft },
	{ L"DtKeyRight", DtKeyRight },
	{ L"DtKeyEnd", DtKeyEnd },
	{ L"DtKeyDown", DtKeyDown },
	{ L"DtKeyNext", DtKeyNext },
	{ L"DtKeyInsert", DtKeyInsert },
	{ L"DtKeyDelete", DtKeyDelete },
	{ L"DtKeyLeftWin", DtKeyLeftWin },
	{ L"DtKeyRightWin", DtKeyRightWin },
	{ L"DtKeyApplications", DtKeyApplications },
	{ L"DtKeyPower", DtKeyPower },
	{ L"DtKeySleep", DtKeySleep },
	{ L"DtKeyWake", DtKeyWake },
	{ L"DtKeyWebSearch", DtKeyWebSearch },
	{ L"DtKeyWebFavorites", DtKeyWebFavorites },
	{ L"DtKeyWebRefresh", DtKeyWebRefresh },
	{ L"DtKeyWebStop", DtKeyWebStop },
	{ L"DtKeyWebForward", DtKeyWebForward },
	{ L"DtKeyWebBack", DtKeyWebBack },
	{ L"DtKeyMyComputer", DtKeyMyComputer },
	{ L"DtKeyMail", DtKeyMail },
	{ L"DtKeyMediaSelect", DtKeyMediaSelect },
	0
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputValueSourceData", 0, InputValueSourceData, ISerializable)

InputValueSourceData::InputValueSourceData()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_index(-1)
{
}

InputValueSourceData::InputValueSourceData(
	InputCategory category,
	InputDefaultControlType controlType,
	const std::wstring& valueId
)
:	m_category(category)
,	m_controlType(controlType)
,	m_index(-1)
,	m_valueId(valueId)
{
}

InputValueSourceData::InputValueSourceData(
	InputCategory category,
	int32_t index,
	InputDefaultControlType controlType,
	const std::wstring& valueId
)
:	m_category(category)
,	m_controlType(controlType)
,	m_index(index)
,	m_valueId(valueId)
{
}

void InputValueSourceData::setCategory(InputCategory category)
{
	m_category = category;
}

InputCategory InputValueSourceData::getCategory() const
{
	return m_category;
}

void InputValueSourceData::setControlType(InputDefaultControlType controlType)
{
	m_controlType = controlType;
}

InputDefaultControlType InputValueSourceData::getControlType() const
{
	return m_controlType;
}

void InputValueSourceData::setIndex(int32_t index)
{
	m_index = index;
}

int32_t InputValueSourceData::getIndex() const
{
	return m_index;
}

void InputValueSourceData::setValueId(const std::wstring& valueId)
{
	m_valueId = valueId;
}

const std::wstring& InputValueSourceData::getValueId() const
{
	return m_valueId;
}

bool InputValueSourceData::serialize(ISerializer& s)
{
	s >> MemberEnum< InputCategory >(L"category", m_category, c_InputCategory_Keys);
	s >> MemberEnum< InputDefaultControlType >(L"controlType", m_controlType, c_InputDefaultControlType_Keys);
	s >> Member< int32_t >(L"index", m_index);
	s >> Member< std::wstring >(L"valueId", m_valueId);
	return true;
}
	
	}
}
