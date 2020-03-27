#pragma once

#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class StaticMesh;

	}

	namespace world
	{

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;

	}

	namespace weather
	{

/*! Precipitation component.
 * \ingroup Weather
 */
class T_DLLCLASS PrecipitationComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	PrecipitationComponent(
		const resource::Proxy< mesh::StaticMesh >& mesh,
		float tiltRate,
		float parallaxDistance,
		float depthDistance,
		float opacity
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass);

private:
	resource::Proxy< mesh::StaticMesh > m_mesh;
	float m_tiltRate;
	float m_parallaxDistance;
	float m_depthDistance;
	float m_opacity;
	Vector4 m_lastEyePosition[4];
	Quaternion m_rotation[4];
	float m_layerAngle[4];
};

	}
}

