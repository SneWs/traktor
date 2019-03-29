#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Illuminate/Editor/Types.h"
#include "Model/Model.h"

namespace traktor
{

class Transform;

	namespace illuminate
	{

class GBuffer : public Object
{
	T_RTTI_CLASS;

public:
	struct Element
	{
		uint32_t polygon;
		uint32_t material;
		Vector4 position;
		Vector4 normal;

		Element()
		:	polygon(model::c_InvalidIndex)
		,	material(model::c_InvalidIndex)
		,	position(Vector4::origo())
		,	normal(Vector4::zero())
		{
		}
	};

	GBuffer();

	bool create(int32_t width, int32_t height, const model::Model& model, const Transform& transform, uint32_t texCoordChannel);

	const Element& get(int32_t x, int32_t y) const { return m_data[x + y * m_width]; }

	void saveAsImages(const std::wstring& outputPath) const;

private:
	int32_t m_width;
	int32_t m_height;
	AlignedVector< Element > m_data;
};

	}
}

