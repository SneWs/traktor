#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Event from target when debug state has changed.
 * \ingroup Amalgam
 */
class T_DLLCLASS ScriptDebuggerStateChange : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStateChange();

	ScriptDebuggerStateChange(bool running);

	bool isRunning() const { return m_running;  }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_running;
};

	}
}

