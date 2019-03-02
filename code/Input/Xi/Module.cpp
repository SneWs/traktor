#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Input/Xi/InputDriverXi.h"

namespace traktor
{
	namespace input
	{

extern "C" void __module__Traktor_Input_Xi()
{
	T_FORCE_LINK_REF(InputDriverXi);
}

	}
}

#endif
