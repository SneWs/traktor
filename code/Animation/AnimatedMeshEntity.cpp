#include <cmath>
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Bone.h"
#include "Animation/IPoseController.h"
#include "Core/Misc/SafeDestroy.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/EntityUpdate.h"

//#define T_USE_UPDATE_JOBS

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshEntity", AnimatedMeshEntity, mesh::MeshEntity)

AnimatedMeshEntity::AnimatedMeshEntity(
	const Transform& transform,
	const resource::Proxy< mesh::SkinnedMesh >& mesh,
	const resource::Proxy< Skeleton >& skeleton,
	IPoseController* poseController,
	const std::vector< int >& boneRemap
)
:	mesh::MeshEntity(transform)
,	m_mesh(mesh)
,	m_skeleton(skeleton)
,	m_poseController(poseController)
,	m_boneRemap(boneRemap)
,	m_totalTime(0.0f)
,	m_updateController(true)
{
}

AnimatedMeshEntity::~AnimatedMeshEntity()
{
	destroy();
}

void AnimatedMeshEntity::destroy()
{
	synchronize();
	safeDestroy(m_poseController);
	mesh::MeshEntity::destroy();
}

Aabb AnimatedMeshEntity::getBoundingBox() const
{
	synchronize();

	Aabb boundingBox = m_mesh.valid() ? m_mesh->getBoundingBox() : Aabb();

	if (!m_poseTransforms.empty())
	{
		for (uint32_t i = 0; i < uint32_t(m_poseTransforms.size()); ++i)
		{
			const Bone* bone = m_skeleton->getBone(i);

			float length = bone->getLength();
			float radius = bone->getRadius();

			Aabb boneLocalAabb(Vector4(-radius, -radius, 0.0f), Vector4(radius, radius, length));
			Aabb boneAabb = boneLocalAabb.transform(m_poseTransforms[i]);

			boundingBox.contain(boneAabb);
		}
	}

	return boundingBox;
}

void AnimatedMeshEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance)
{
	if (!m_mesh.validate())
		return;

	synchronize();

	m_mesh->render(
		worldContext->getRenderContext(),
		worldRenderView,
		m_transform,
		m_skinTransforms,
		distance,
		getParameterCallback()
	);

	m_first = false;
	m_updateController = true;
}

void AnimatedMeshEntity::update(const world::EntityUpdate* update)
{
	if (!m_skeleton.validate() || !m_updateController)
		return;

	synchronize();

	// Prevent further updates from evaluating pose controller,
	// each pose controller needs to set this flag if it's
	// required to continue running even when this entity
	// hasn't been rendered.
	m_updateController = false;

#if defined(T_USE_UPDATE_JOBS)
	m_updatePoseControllerJob = makeFunctor< AnimatedMeshEntity, float >(
		this,
		&AnimatedMeshEntity::updatePoseController,
		update->getDeltaTime()
	);
	JobManager::getInstance().add(m_updatePoseControllerJob);
#else
	updatePoseController(update->getDeltaTime());
#endif
}

void AnimatedMeshEntity::setTransform(const Transform& transform)
{
	// Let pose controller know that entity has been manually repositioned.
	if (m_poseController)
		m_poseController->setTransform(transform);

	mesh::MeshEntity::setTransform(transform);
}

bool AnimatedMeshEntity::getBoneTransform(const std::wstring& boneName, Transform& outTransform) const
{
	uint32_t index;

	synchronize();

	if (!m_skeleton.validate() || !m_skeleton->findBone(boneName, index))
		return false;
	if (index >= m_boneTransforms.size())
		return false;

	outTransform = m_boneTransforms[index];
	return true;
}

bool AnimatedMeshEntity::getPoseTransform(const std::wstring& boneName, Transform& outTransform) const
{
	uint32_t index;

	synchronize();

	if (!m_skeleton.validate() || !m_skeleton->findBone(boneName, index))
		return false;
	if (index >= m_poseTransforms.size())
		return false;

	outTransform = m_poseTransforms[index];
	return true;
}

bool AnimatedMeshEntity::getSkinTransform(const std::wstring& boneName, Matrix44& outTransform) const
{
	uint32_t index;

	synchronize();

	if (!m_skeleton.validate() || !m_skeleton->findBone(boneName, index))
		return false;
	if (index >= m_poseTransforms.size())
		return false;

	int skinIndex = m_boneRemap[index];
	if (skinIndex < 0)
		return false;

	outTransform = m_skinTransforms[skinIndex];
	return true;
}

void AnimatedMeshEntity::synchronize() const
{
#if defined(T_USE_UPDATE_JOBS)
	m_updatePoseControllerJob.wait();
	m_updatePoseControllerJob = 0;
#endif
}

void AnimatedMeshEntity::updatePoseController(float deltaTime)
{
	if (m_poseController)
	{
		m_boneTransforms.resize(0);
		m_poseTransforms.resize(0);

		calculateBoneTransforms(
			m_skeleton,
			m_boneTransforms
		);

		m_poseController->evaluate(
			deltaTime,
			m_transform,
			m_skeleton,
			m_boneTransforms,
			m_poseTransforms,
			m_updateController
		);

		size_t skeletonBoneCount = m_boneTransforms.size();
		size_t skinBoneCount = m_mesh->getBoneCount();

		while (m_poseTransforms.size() < skeletonBoneCount)
			m_poseTransforms.push_back(Transform::identity());

		m_skinTransforms.resize(skinBoneCount);
		for (size_t i = 0; i < skinBoneCount; ++i)
			m_skinTransforms[i] = Matrix44::identity();

		for (size_t i = 0; i < skeletonBoneCount; ++i)
		{
			int32_t boneIndex = m_boneRemap[i];
			if (boneIndex >= 0 && boneIndex < int32_t(skinBoneCount))
				m_skinTransforms[boneIndex] = (m_poseTransforms[i] * m_boneTransforms[i].inverse()).toMatrix44();
		}
	}
	else
	{
		size_t skinBoneCount = m_mesh->getBoneCount();
		m_skinTransforms.resize(skinBoneCount);
		for (size_t i = 0; i < skinBoneCount; ++i)
			m_skinTransforms[i] = Matrix44::identity();
	}
}

	}
}
