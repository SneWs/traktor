#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class SHMatrix;

/*! Spherical harmonics coefficients.
 * \ingroup Render
 */
class T_DLLCLASS SHCoeffs : public ISerializable
{
	T_RTTI_CLASS;

public:
	void resize(size_t coefficientCount);

	bool empty() const;

	SHCoeffs transform(const SHMatrix& matrix) const;

	Vector4 operator * (const SHCoeffs& coeffs) const;

	Vector4& operator [] (uint32_t index) { return m_data[index]; }

	const Vector4& operator [] (uint32_t index) const { return m_data[index]; }

	const AlignedVector< Vector4 >& get() const { return m_data; }

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Vector4 > m_data;
};

	}
}
