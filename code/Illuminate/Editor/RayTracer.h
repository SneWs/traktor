#pragma once

#include "Core/Object.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Illuminate/Editor/KdTree.h"
#include "Illuminate/Editor/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

class Model;

	}

	namespace illuminate
	{

class IlluminateConfiguration;

class T_DLLCLASS RayTracer : public Object
{
	T_RTTI_CLASS;

public:
	explicit RayTracer(const IlluminateConfiguration* configuration);

	void addLight(const Light& light);

	void addModel(const model::Model* model, const Transform& transform);

	bool prepare();

	Color4f traceDirect(const Vector4& origin, const Vector4& normal, float roughness);

	Color4f traceIndirect(const Vector4& origin, const Vector4& normal, float roughness);

	Scalar traceOcclusion(const Vector4& origin, const Vector4& normal);

private:
	struct Photon
	{
		Vector4 position;
		Vector4 direction;	//!< Incoming direction
		Color4f intensity;

		operator const Vector4& () const { return position; }
	};

	const IlluminateConfiguration* m_configuration;
	SahTree m_sah;
	SahTree::QueryCache m_sahCache;
	KdTree< Photon > m_photonMap;
	AlignedVector< Light > m_lights;
	AlignedVector< Winding3 > m_windings;
	RandomGeometry m_random;
	float m_maxDistance;

	Color4f sampleAnalyticalLights(const Vector4& origin, const Vector4& normal, uint32_t shadowSampleCount, float pointLightShadowRadius);
};

	}
}
