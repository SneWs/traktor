#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix33.h"
#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! Convolution filter.
 * \ingroup Drawing
 */
class T_DLLCLASS ConvolutionFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit ConvolutionFilter(int32_t size);

	explicit ConvolutionFilter(const float* matrix, int32_t size);

	static Ref< ConvolutionFilter > createGaussianBlur3();

	static Ref< ConvolutionFilter > createGaussianBlur5();

	static Ref< ConvolutionFilter > createGaussianBlur(int32_t radius);

	static Ref< ConvolutionFilter > createEmboss();

protected:
	virtual void apply(Image* image) const override final;

private:
	AlignedVector< Scalar > m_matrix;
	int32_t m_size;
};

	}
}

