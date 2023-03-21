/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Display.h"
#include "Ui/Size.h"

namespace traktor::ui
{

/*! 2D point.
 * \ingroup UI
 */
class Point
{
public:
	int32_t x;
	int32_t y;

	inline Point();

	inline Point(const Point& pnt);

	inline Point(const std::pair< int32_t, int32_t >& pr);

	inline Point(int32_t x_, int32_t y_);

	inline Point dpi96() const;

	inline Point invdpi96() const;

	inline Point operator + (const Size& s) const;

	inline Point operator - (const Size& s) const;

	inline const Point& operator += (const Size& s);

	inline const Point& operator -= (const Size& s);

	inline Size operator - (const Point& p) const;

	inline bool operator == (const Point& p) const;

	inline bool operator != (const Point& p) const;
};

}

#include "Ui/Point.inl"
