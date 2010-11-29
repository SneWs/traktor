#ifndef traktor_world_DirectionalLightEntity_H
#define traktor_world_DirectionalLightEntity_H

#include "World/Entity/SpatialEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Directional light entity.
 * \ingroup World
 */
class T_DLLCLASS DirectionalLightEntity : public SpatialEntity
{
	T_RTTI_CLASS;

public:
	DirectionalLightEntity(
		const Transform& transform,
		const Vector4& sunColor,
		const Vector4& baseColor,
		const Vector4& shadowColor
	);

	virtual void update(const EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb getBoundingBox() const;

	void setSunColor(const Vector4& sunColor) { m_sunColor = sunColor; }

	const Vector4& getSunColor() const { return m_sunColor; }

	void setBaseColor(const Vector4& baseColor) { m_baseColor = baseColor; }

	const Vector4& getBaseColor() const { return m_baseColor; }

	void setShadowColor(const Vector4& shadowColor) { m_shadowColor = shadowColor; }

	const Vector4& getShadowColor() const { return m_shadowColor; }

private:
	Transform m_transform;
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
};

	}
}

#endif	// traktor_world_DirectionalLightEntity_H
