#pragma once

#include "World/IWorldShadowProjection.h"

namespace traktor
{
	namespace world
	{

class UniformShadowProjection : public IWorldShadowProjection
{
	T_RTTI_CLASS;

public:
	UniformShadowProjection(uint32_t realShadowMapSize);

	virtual void calculate(
		const Matrix44& viewInverse,
		const Vector4& lightPosition,
		const Vector4& lightDirection,
		const Frustum& viewFrustum,
		const Aabb3& shadowBox,
		float shadowFarZ,
		bool quantizeProjection,
		Matrix44& outLightView,
		Matrix44& outLightProjection,
		Frustum& outShadowFrustum
	) const override final;

private:
	float m_realShadowMapSize;
};

	}
}

