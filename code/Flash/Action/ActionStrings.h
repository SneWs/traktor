#pragma once

#include <map>
#include <string>
#include "Core/Object.h"

namespace traktor
{
	namespace flash
	{

class ActionStrings : public Object
{
	T_RTTI_CLASS;

public:
	ActionStrings();

	uint32_t operator [] (const std::string& str);

	const std::string& operator [] (uint32_t id) const;

private:
	std::map< std::string, uint32_t > m_strings;
	uint32_t m_next;
};

	}
}

