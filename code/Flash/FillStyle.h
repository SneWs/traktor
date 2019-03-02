#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix33.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace flash
	{

class ColorTransform;

#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif

/*! \brief Shape fill style.
 * \ingroup Flash
 */
class T_DLLCLASS FillStyle
{
public:
	enum GradientType
	{
		GtInvalid,
		GtLinear,
		GtRadial
	};

	struct ColorRecord
	{
		float ratio;
		Color4f color;

		ColorRecord()
		{
		}

		ColorRecord(float ratio_, const Color4f& color_)
		:	ratio(ratio_)
		,	color(color_)
		{
		}

		void serialize(ISerializer& s);
	};

	FillStyle();

	bool create(const SwfFillStyle* fillStyle);

	bool create(const Color4f& solidColor);

	bool create(GradientType gradientType, const AlignedVector< ColorRecord >& colorRecords, const Matrix33& gradientMatrix);

	bool create(uint16_t fillBitmap, const Matrix33& fillBitmapMatrix, bool fillBitmapRepeat);

	void transform(const Matrix33& transform, const ColorTransform& cxform);

	const AlignedVector< ColorRecord >& getColorRecords() const { return m_colorRecords; }

	GradientType getGradientType() const { return m_gradientType; }

	const Matrix33& getGradientMatrix() const { return m_gradientMatrix; }

	uint16_t getFillBitmap() const { return m_fillBitmap; }

	const Matrix33& getFillBitmapMatrix() const { return m_fillBitmapMatrix; }

	bool getFillBitmapRepeat() const { return m_fillBitmapRepeat; }

	void serialize(ISerializer& s);

private:
	AlignedVector< ColorRecord > m_colorRecords;
	GradientType m_gradientType;
	Matrix33 m_gradientMatrix;
	uint16_t m_fillBitmap;
	Matrix33 m_fillBitmapMatrix;
	bool m_fillBitmapRepeat;
};

#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

	}
}

