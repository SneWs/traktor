#ifndef traktor_traktor_amalgam_I18NClasses_H
#define traktor_traktor_amalgam_I18NClasses_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptManager;

	}

	namespace amalgam
	{

void T_DLLCLASS registerI18NClasses(script::IScriptManager* scriptManager);

	}
}

#endif	// traktor_traktor_amalgam_I18NClasses_H
