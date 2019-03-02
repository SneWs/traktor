#pragma once

#include "Core/Math/Plane.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Spray/Types.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class ISoundPlayer;

	}

	namespace world
	{

class IEntityEventManager;

	}

	namespace spray
	{

class Effect;
class EffectInstance;
class MeshRenderer;
class PointRenderer;
class TrailRenderer;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS EffectComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	EffectComponent(const resource::Proxy< Effect >& effect, world::IEntityEventManager* eventManager, sound::ISoundPlayer* soundPlayer);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void render(
		render::handle_t technique,
		const Vector4& cameraPosition,
		const Plane& cameraPlane,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer
	);

	Aabb3 getWorldBoundingBox() const;

	void setLoopEnable(bool loopEnable);

	bool getLoopEnable() const;

	bool isFinished() const;

	const resource::Proxy< Effect >& getEffect() const { return m_effect; }

	void setEnable(bool enable) { m_enable = enable; }

	bool isEnable() const { return m_enable; }

	bool haveTechnique(render::handle_t technique) const { return m_techniques.find(technique) != m_techniques.end(); }

	void setVirtualSourceCallback(VirtualSourceCallback* virtualSourceCallback) { m_context.virtualSourceCallback = virtualSourceCallback; }

private:
	Transform m_transform;
	resource::Proxy< Effect > m_effect;
	Ref< EffectInstance > m_effectInstance;
	std::set< render::handle_t > m_techniques;
	Context m_context;
	uint32_t m_counter;
	bool m_enable;
};

	}
}

