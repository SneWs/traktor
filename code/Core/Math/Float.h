#pragma once

namespace traktor
{

/*! \ingroup Core */
//@{

inline bool isNan(float value)
{
	unsigned int bits = *reinterpret_cast< const unsigned int* >(&value);
	return ((bits & 0x7f800000) >> 23) == 0xff && (bits & 0x7fffff) != 0;
}

inline bool isInfinite(float value)
{
	unsigned int bits = *reinterpret_cast< const unsigned int* >(&value);
	return (bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) == 0;
}

inline bool isNanOrInfinite(float value)
{
	return isNan(value) || isInfinite(value);
}

inline float lerp(float a, float b, float c)
{
	return a * (1.0f - c) + b * c;
}

//@}

}

