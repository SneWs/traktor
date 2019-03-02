#pragma once

namespace traktor
{
	namespace input
	{

/*! \brief InputDefaultControlType to DIK mapping. */
const DWORD c_di8ControlKeys[] =
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	DIK_ESCAPE,
	DIK_1,
	DIK_2,
	DIK_3,
	DIK_4,
	DIK_5,
	DIK_6,
	DIK_7,
	DIK_8,
	DIK_9,
	DIK_0,
	DIK_MINUS,
	DIK_EQUALS,
	DIK_BACK,
	DIK_TAB,
	DIK_Q,
	DIK_W,
	DIK_E,
	DIK_R,
	DIK_T,
	DIK_Y,
	DIK_U,
	DIK_I,
	DIK_O,
	DIK_P,
	DIK_LBRACKET,
	DIK_RBRACKET,
	DIK_RETURN,
	DIK_LCONTROL,
	DIK_A,
	DIK_S,
	DIK_D,
	DIK_F,
	DIK_G,
	DIK_H,
	DIK_J,
	DIK_K,
	DIK_L,
	DIK_SEMICOLON,
	DIK_APOSTROPHE,
	DIK_GRAVE,
	DIK_LSHIFT,
	DIK_BACKSLASH,
	DIK_Z,
	DIK_X,
	DIK_C,
	DIK_V,
	DIK_B,
	DIK_N,
	DIK_M,
	DIK_COMMA,
	DIK_PERIOD,
	DIK_SLASH,
	DIK_RSHIFT,
	DIK_MULTIPLY,
	DIK_LMENU,
	DIK_SPACE,
	DIK_CAPITAL,
	DIK_F1,
	DIK_F2,
	DIK_F3,
	DIK_F4,
	DIK_F5,
	DIK_F6,
	DIK_F7,
	DIK_F8,
	DIK_F9,
	DIK_F10,
	DIK_NUMLOCK,
	DIK_SCROLL,
	DIK_NUMPAD7,
	DIK_NUMPAD8,
	DIK_NUMPAD9,
	DIK_SUBTRACT,
	DIK_NUMPAD4,
	DIK_NUMPAD5,
	DIK_NUMPAD6,
	DIK_ADD,
	DIK_NUMPAD1,
	DIK_NUMPAD2,
	DIK_NUMPAD3,
	DIK_NUMPAD0,
	DIK_DECIMAL,
	DIK_OEM_102,
	DIK_F11,
	DIK_F12,
	DIK_F13,
	DIK_F14,
	DIK_F15,
	DIK_KANA,
	DIK_ABNT_C1,
	DIK_CONVERT,
	DIK_NOCONVERT,
	DIK_YEN,
	DIK_ABNT_C2,
	DIK_NUMPADEQUALS,
	DIK_PREVTRACK,
	DIK_AT,
	DIK_COLON,
	DIK_UNDERLINE,
	DIK_KANJI,
	DIK_STOP,
	DIK_AX,
	DIK_UNLABELED,
	DIK_NEXTTRACK,
	DIK_NUMPADENTER,
	DIK_RCONTROL,
	DIK_MUTE,
	DIK_CALCULATOR,
	DIK_PLAYPAUSE,
	DIK_MEDIASTOP,
	DIK_VOLUMEDOWN,
	DIK_VOLUMEUP,
	DIK_WEBHOME,
	DIK_NUMPADCOMMA,
	DIK_DIVIDE,
	DIK_SYSRQ,
	DIK_RMENU,
	DIK_PAUSE,
	DIK_HOME,
	DIK_UP,
	DIK_PRIOR,
	DIK_LEFT,
	DIK_RIGHT,
	DIK_END,
	DIK_DOWN,
	DIK_NEXT,
	DIK_INSERT,
	DIK_DELETE,
	DIK_LWIN,
	DIK_RWIN,
	DIK_APPS,
	DIK_POWER,
	DIK_SLEEP,
	DIK_WAKE,
	DIK_WEBSEARCH,
	DIK_WEBFAVORITES,
	DIK_WEBREFRESH,
	DIK_WEBSTOP,
	DIK_WEBFORWARD,
	DIK_WEBBACK,
	DIK_MYCOMPUTER,
	DIK_MAIL,
	DIK_MEDIASELECT
};

/*! \brief InputDefaultControlType to VK mapping. */
const DWORD c_vkControlKeys[] =
{
	0,	// DtInvalid
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,	// 10
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,	// 20
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,	// 30
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	VK_ESCAPE,	// 40
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'0',
	'-',
	'=',
	VK_BACK,
	VK_TAB,
	'Q',
	'W',
	'E',
	'R',
	'T',
	'Y',
	'U',
	'I',
	'O',
	'P',
	'[',
	']',
	VK_RETURN,
	VK_LCONTROL,
	'A',
	'S',
	'D',
	'F',
	'G',
	'H',
	'J',
	'K',
	'L',
	';',
	'\"',
	0/*'\''*/,
	VK_LSHIFT,
	'\\',
	'Z',
	'X',
	'C',
	'V',
	'B',
	'N',
	'M',
	',',
	'.',
	'/',
	VK_RSHIFT,
	VK_MULTIPLY,
	VK_LMENU,
	VK_SPACE,
	0/*VK_CAPITAL*/,
	VK_F1,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	0/*VK_NUMLOCK*/,
	VK_SCROLL,
	VK_NUMPAD7,
	VK_NUMPAD8,
	VK_NUMPAD9,
	VK_SUBTRACT,
	VK_NUMPAD4,
	VK_NUMPAD5,
	VK_NUMPAD6,
	VK_ADD,
	VK_NUMPAD1,
	VK_NUMPAD2,
	VK_NUMPAD3,
	VK_NUMPAD0,
	VK_DECIMAL,
	VK_OEM_102,
	VK_F11,
	VK_F12,
	VK_F13,
	VK_F14,
	VK_F15,
	VK_KANA,
	0,
	VK_CONVERT,
	0,
	0,
	0,
	0,
	0,
	'@',
	':',
	'_',
	VK_KANJI,
	0,
	0,
	0,
	0,
	0,
	VK_RCONTROL,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	VK_DIVIDE,
	0,
	VK_RMENU,
	VK_PAUSE,
	VK_HOME,
	VK_UP,
	VK_PRIOR,
	VK_LEFT,
	VK_RIGHT,
	VK_END,
	VK_DOWN,
	VK_NEXT,
	VK_INSERT,
	VK_DELETE,
	VK_LWIN,
	VK_RWIN,
	VK_APPS,
	0,
	VK_SLEEP,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

uint32_t translateFromVk(uint32_t vk);

	}
}

