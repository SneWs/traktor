#include "Render/Shader.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterInstance.h"
#include "Spray/Modifier.h"
#include "Spray/Source.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Emitter", Emitter, Object)

Emitter::Emitter(
	Source* source,
	const RefArray< Modifier >& modifiers,
	const resource::Proxy< render::Shader >& shader,
	const resource::Proxy< mesh::InstanceMesh >& mesh,
	float middleAge,
	float cullNearDistance,
	float cullMeshDistance,
	float fadeNearRange,
	float warmUp,
	bool sort,
	bool worldSpace,
	bool meshOrientationFromVelocity
)
:	m_source(source)
,	m_modifiers(modifiers)
,	m_shader(shader)
,	m_mesh(mesh)
,	m_middleAge(middleAge)
,	m_cullNearDistance(cullNearDistance)
,	m_cullMeshDistance(cullMeshDistance)
,	m_fadeNearRange(fadeNearRange)
,	m_warmUp(warmUp)
,	m_sort(sort)
,	m_worldSpace(worldSpace)
,	m_meshOrientationFromVelocity(meshOrientationFromVelocity)
{
}

Ref< EmitterInstance > Emitter::createInstance(float duration) const
{
	return new EmitterInstance(this, duration);
}

	}
}
