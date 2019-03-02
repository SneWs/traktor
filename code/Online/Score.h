#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class IUser;

class T_DLLCLASS Score : public Object
{
	T_RTTI_CLASS;

public:
	Score(const IUser* user, int32_t score, uint32_t rank);

	const IUser* getUser() const { return m_user; }

	int32_t getScore() const { return m_score; }

	uint32_t getRank() const { return m_rank; }

private:
	Ref< const IUser > m_user;
	int32_t m_score;
	uint32_t m_rank;
};

	}
}

