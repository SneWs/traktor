#pragma once

#include <map>
#include <vector>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS RecordInputScript : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addInputValue(uint32_t frame, int control, float value);

	float getInputValue(uint32_t frame, int control);

	uint32_t getLastFrame() const;

	virtual void serialize(ISerializer& s) override final;

private:
	struct Input
	{
		uint32_t start;
		uint32_t end;
		float value;

		void serialize(ISerializer& s);
	};

	std::map< int, std::vector< Input > > m_data;
};

	}
}

