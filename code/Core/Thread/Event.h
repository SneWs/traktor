/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Thread signal event.
 * \ingroup Core
 */
class T_DLLCLASS Event : public IWaitable
{
public:
	Event();

	virtual ~Event();

	void pulse(int32_t count = 1);

	void broadcast();

	void reset();

	virtual bool wait(int32_t timeout = -1);

private:
	void* m_handle;
};

}

