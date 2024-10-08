/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace traktor
{

/*! Scoped value.
 * \ingroup Core
 *
 * Automatic restoration of variable values when getting out of scope.
 * Useful when a function has several return points.
 * \example
 * {
 *    Save< int > saveFoo(g_foo);
 *    g_foo = 2;
 *    // after function finished g_foo will be restored to it's original value.
 * }
 */
template < typename T >
class Save
{
public:
	Save(T& var)
	:	m_ref(var)
	,	m_val(var)
	{
	}

	Save(T& var, T value)
	:	m_ref(var)
	,	m_val(var)
	{
		m_ref = value;
	}

	~Save()
	{
		m_ref = m_val;
	}

private:
	T& m_ref;
	T m_val;
};

}

