#include "Spray/Sources/QuadSource.h"
#include "Spray/EmitterUpdateContext.h"
#include "Spray/EmitterInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.QuadSource", 0, QuadSource, Source)

QuadSource::QuadSource()
:	m_center(0.0f, 0.0f, 0.0f, 1.0f)
,	m_axis1(1.0f, 0.0f, 0.0f, 0.0f)
,	m_axis2(0.0f, 0.0f, 1.0f, 0.0f)
,	m_normal(0.0f, 1.0f, 0.0f, 0.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

bool QuadSource::create(resource::IResourceManager* resourceManager)
{
	return true;
}

void QuadSource::emit(
	EmitterUpdateContext& context,
	const Transform& transform,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	Vector4 center = transform * m_center;
	Vector4 axis1 = transform * m_axis1;
	Vector4 axis2 = transform * m_axis2;
	Vector4 normal = transform * m_normal;

	while (emitCount-- > 0)
	{
		Scalar u = Scalar(float(context.random.nextDouble()) * 2.0f - 1.0f);
		Scalar v = Scalar(float(context.random.nextDouble()) * 2.0f - 1.0f);

		Point point;

		point.position = center + u * axis1 + v * axis2;
		point.velocity = normal * Scalar(m_velocity.random(context.random));
		point.orientation = m_orientation.random(context.random);
		point.angularVelocity = m_angularVelocity.random(context.random);
		point.color = Vector4::one();
		point.age = 0.0f;
		point.maxAge = m_age.random(context.random);
		point.inverseMass = 1.0f / (m_mass.random(context.random));
		point.size = m_size.random(context.random);
		point.random = context.random.nextFloat();

		emitterInstance.addPoint(point);
	}
}

bool QuadSource::serialize(ISerializer& s)
{
	if (!Source::serialize(s))
		return false;

	s >> Member< Vector4 >(L"center", m_center);
	s >> Member< Vector4 >(L"axis1", m_axis1);
	s >> Member< Vector4 >(L"axis2", m_axis2);
	s >> Member< Vector4 >(L"normal", m_normal);
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);

	return true;
}

	}
}
