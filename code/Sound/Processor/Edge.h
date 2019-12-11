#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class InputPin;
class OutputPin;

/*! Shader graph edge.
 * \ingroup Render
 */
class T_DLLCLASS Edge : public ISerializable
{
	T_RTTI_CLASS;

public:
	Edge(const OutputPin* source = 0, const InputPin* destination = 0);

	void setSource(const OutputPin* source);

	const OutputPin* getSource() const;

	void setDestination(const InputPin* destination);

	const InputPin* getDestination() const;

	virtual void serialize(ISerializer& s) override final;

private:
	const OutputPin* m_source;
	const InputPin* m_destination;
};

	}
}
