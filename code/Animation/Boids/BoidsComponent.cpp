/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Boids/BoidsComponent.h"
#include "Core/Math/RandomGeometry.h"
#include "World/Entity.h"
#include "World/WorldBuildContext.h"

namespace traktor::animation
{
	namespace
	{

RandomGeometry s_random;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsComponent", BoidsComponent, world::IEntityComponent)

BoidsComponent::BoidsComponent(
	const RefArray< world::Entity >& boidEntities,
	const Vector4& spawnPositionDiagonal,
	const Vector4& spawnVelocityDiagonal,
	const Vector4& constrain,
	float followForce,
	float repelDistance,
	float repelForce,
	float matchVelocityStrength,
	float centerForce,
	float maxVelocity
)
:	m_boidEntities(boidEntities)
,	m_constrain(constrain)
,	m_followForce(followForce)
,	m_repelDistance(repelDistance)
,	m_repelForce(repelForce)
,	m_matchVelocityStrength(matchVelocityStrength)
,	m_centerForce(centerForce)
,	m_maxVelocity(maxVelocity)
{
	m_boids.resize(m_boidEntities.size());
	for (auto& boid : m_boids)
	{
		boid.position = m_transform * (s_random.nextUnit() * spawnPositionDiagonal).xyz1();
		boid.velocity = s_random.nextUnit() * spawnVelocityDiagonal;
	}
}

void BoidsComponent::destroy()
{
	for (auto boidEntity : m_boidEntities)
	{
		if (boidEntity)
			boidEntity->destroy();
	}
	m_boidEntities.resize(0);
}

void BoidsComponent::setOwner(world::Entity* owner)
{
}

void BoidsComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 BoidsComponent::getBoundingBox() const
{
	const Transform transformInv = m_transform.inverse();

	Aabb3 aabb;
	for (uint32_t i = 0; i < uint32_t(m_boids.size()); ++i)
		aabb.contain(transformInv * m_boids[i].position);

	return aabb;
}

void BoidsComponent::update(const world::UpdateParams& update)
{
	const Scalar deltaTime(min(update.deltaTime, 1.0f / 30.0f));
	const Vector4 attraction = m_transform.translation().xyz1();

	// Calculate perceived center and velocity of all boids.
	Vector4 center(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4 velocity(0.0f, 0.0f, 0.0f, 0.0f);
	for (uint32_t i = 0; i < m_boids.size(); ++i)
	{
		center += m_boids[i].position;
		velocity += m_boids[i].velocity;
	}

	const Vector4 constrainAdd = attraction * (Vector4::one() - m_constrain);
	const Scalar invBoidsSize(1.0f / (float(m_boids.size()) - 1.0f));

	// Update boids.
	for (uint32_t i = 0; i < m_boids.size(); ++i)
	{
		const Vector4 otherCenter = (center - m_boids[i].position) * invBoidsSize;
		const Vector4 otherVelocity = (velocity - m_boids[i].velocity) * invBoidsSize;

		// 1: Follow perceived center.
		m_boids[i].velocity += (otherCenter - m_boids[i].position) * m_followForce;

		// 2: Keep distance from other boids.
		for (uint32_t j = 0; j < uint32_t(m_boids.size()); ++j)
		{
			if (i != j)
			{
				Vector4 d = m_boids[j].position - m_boids[i].position;
				if (d.normalize() < m_repelDistance)
					m_boids[i].velocity -= d * m_repelForce;
			}
		}

		// 3: Try to match velocity with other boids.
		m_boids[i].velocity += (otherVelocity - m_boids[i].velocity) * m_matchVelocityStrength;

		// 4: Always try to be circulating around center.
		m_boids[i].velocity += (attraction - m_boids[i].position).xyz0() * m_centerForce;

		// 5: Clamp velocity.
		const Scalar ln = m_boids[i].velocity.length();
		m_boids[i].velocity = m_boids[i].velocity.normalized() * min(ln, m_maxVelocity);

		// Integrate position.
		m_boids[i].position += m_boids[i].velocity * deltaTime;

		// Constrain position.
		m_boids[i].position = m_boids[i].position * m_constrain + constrainAdd;

		// Update boid entity.
		if (m_boidEntities[i])
		{
			m_boidEntities[i]->setTransform(Transform(
				lookAt(m_boids[i].position, m_boids[i].position + m_boids[i].velocity).inverse()
			));
			m_boidEntities[i]->update(update);
		}
	}
}

}
