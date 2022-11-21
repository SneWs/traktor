/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <limits>
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Heightfield/Heightfield.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::terrain
{
	namespace
	{

const render::Handle c_handleTerrain_VisualizeLods(L"Terrain_VisualizeLods");
const render::Handle c_handleTerrain_VisualizeMap(L"Terrain_VisualizeMap");
const render::Handle c_handleTerrain_SurfaceAlbedo(L"Terrain_SurfaceAlbedo");
const render::Handle c_handleTerrain_SurfaceNormals(L"Terrain_SurfaceNormals");
const render::Handle c_handleTerrain_SurfaceOffset(L"Terrain_SurfaceOffset");
const render::Handle c_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle c_handleTerrain_ColorMap(L"Terrain_ColorMap");
const render::Handle c_handleTerrain_SplatMap(L"Terrain_SplatMap");
const render::Handle c_handleTerrain_CutMap(L"Terrain_CutMap");
const render::Handle c_handleTerrain_MaterialMap(L"Terrain_MaterialMap");
const render::Handle c_handleTerrain_Normals(L"Terrain_Normals");
const render::Handle c_handleTerrain_Eye(L"Terrain_Eye");
const render::Handle c_handleTerrain_WorldOrigin(L"Terrain_WorldOrigin");
const render::Handle c_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle c_handleTerrain_PatchOrigin(L"Terrain_PatchOrigin");
const render::Handle c_handleTerrain_PatchExtent(L"Terrain_PatchExtent");
const render::Handle c_handleTerrain_DebugPatchIndex(L"Terrain_DebugPatchIndex");
const render::Handle c_handleTerrain_DebugMap(L"Terrain_DebugMap");
const render::Handle c_handleTerrain_CutEnable(L"Terrain_CutEnable");
const render::Handle c_handleTerrain_ColorEnable(L"Terrain_ColorEnable");

const int32_t c_patchLodSteps = 3;
const int32_t c_surfaceLodSteps = 3;

struct CullPatch
{
	float error[4];
	float distance;
	float area;
	uint32_t patchId;
	Vector4 patchOrigin;
};

typedef std::pair< float, const TerrainComponent::Patch* > cull_patch_t;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainComponent", TerrainComponent, world::IEntityComponent)

TerrainComponent::TerrainComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_owner(nullptr)
,	m_cacheSize(0)
,	m_visualizeMode(VmDefault)
{
}

bool TerrainComponent::create(const TerrainComponentData& data)
{
	if (!m_resourceManager->bind(data.getTerrain(), m_terrain))
		return false;

	m_heightfield = m_terrain->getHeightfield();

	m_patchLodDistance = data.getPatchLodDistance();
	m_patchLodBias = data.getPatchLodBias();
	m_patchLodExponent = data.getPatchLodExponent();
	m_surfaceLodDistance = data.getSurfaceLodDistance();
	m_surfaceLodBias = data.getSurfaceLodBias();
	m_surfaceLodExponent = data.getSurfaceLodExponent();

	if (!createPatches())
		return false;

	return true;
}

void TerrainComponent::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	float detailDistance,
	uint32_t cacheSize
)
{
	const int32_t viewIndex = worldRenderView.getIndex();
	const bool snapshot = worldRenderView.getSnapshot();

	if (!validate(viewIndex, cacheSize))
		return;

	const Vector4& worldExtent = m_heightfield->getWorldExtent();

	const Matrix44 viewInv = worldRenderView.getView().inverse();
	const Matrix44 viewProj = worldRenderView.getProjection() * worldRenderView.getView();
	const Vector4 eyePosition = worldRenderView.getEyePosition();
	const Vector4 eyeDirection = worldRenderView.getEyeDirection();

	const Vector4 patchExtent(worldExtent.x() / float(m_patchCount), worldExtent.y(), worldExtent.z() / float(m_patchCount), 0.0f);
	const Vector4 patchDeltaHalf = patchExtent * Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	const Vector4 patchDeltaX = patchExtent * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	const Vector4 patchDeltaZ = patchExtent * Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 patchTopLeft = (-worldExtent * Scalar(0.5f)).xyz1();

	// Calculate world frustum.
	const Frustum viewCullFrustum = worldRenderView.getCullFrustum();
	Frustum worldCullFrustum = viewCullFrustum;
	for (uint32_t i = 0; i < worldCullFrustum.planes.size(); ++i)
		worldCullFrustum.planes[i] = viewInv * worldCullFrustum.planes[i];

	// Cull patches to world frustum.
	m_visiblePatches.resize(0);
	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		Vector4 patchOrigin = patchTopLeft;
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
			const uint32_t patchId = px + pz * m_patchCount;

			const Patch& patch = m_patches[patchId];
			const Vector4 patchCenterWorld = (patchOrigin + patchDeltaHalf) * Vector4(1.0f, 0.0f, 1.0f, 0.0f) + Vector4(0.0f, (patch.minHeight + patch.maxHeight) * 0.5f, 0.0f, 1.0f);

			const Aabb3 patchAabb(
				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4(-patchDeltaHalf.x(), patch.minHeight - FUZZY_EPSILON, -patchDeltaHalf.z(), 0.0f),
				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4( patchDeltaHalf.x(), patch.maxHeight + FUZZY_EPSILON,  patchDeltaHalf.z(), 0.0f)
			);

			if (worldCullFrustum.inside(patchAabb) != Frustum::IrOutside)
			{
				const Scalar lodDistance = (patchCenterWorld - eyePosition).xyz0().length();
				const Vector4 patchCenterWorld_x0zw = patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f);
				const Vector4 eyePosition_0y00 = Vector4(0.0f, eyePosition.y(), 0.0f, 0.0f);

				CullPatch cp;

				// Calculate screen error for each lod.
				for (int i = 0; i < LodCount; ++i)
				{
					const Vector4 Pworld[2] =
					{
						patchCenterWorld_x0zw + eyePosition_0y00,
						patchCenterWorld_x0zw + eyePosition_0y00 + Vector4(0.0f, patch.error[i], 0.0f, 0.0f)
					};

					Vector4 Pview[2] =
					{
						worldRenderView.getView() * Pworld[0],
						worldRenderView.getView() * Pworld[1]
					};

					if (Pview[0].z() < viewCullFrustum.getNearZ())
						Pview[0].set(2, viewCullFrustum.getNearZ());
					if (Pview[1].z() < viewCullFrustum.getNearZ())
						Pview[1].set(2, viewCullFrustum.getNearZ());

					Vector4 Pclip[] =
					{
						worldRenderView.getProjection() * Pview[0].xyz1(),
						worldRenderView.getProjection() * Pview[1].xyz1()
					};

					T_ASSERT(Pclip[0].w() > 0.0f);
					T_ASSERT(Pclip[1].w() > 0.0f);

					Pclip[0] /= Pclip[0].w();
					Pclip[1] /= Pclip[1].w();

					const Vector4 d = Pclip[1] - Pclip[0];

					const float dx = d.x();
					const float dy = d.y();

					cp.error[i] = std::sqrt(dx * dx + dy * dy) * 100.0f;
				}

				// Project patch bounding box extents onto view plane and calculate screen area.
				Vector4 extents[8];
				patchAabb.getExtents(extents);

				Vector4 mn(
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max()
				);
				Vector4 mx(
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max()
				);

				bool clipped = false;
				for (int32_t i = 0; i < sizeof_array(extents); ++i)
				{
					Vector4 p = viewProj * extents[i];
					if (p.w() <= 0.0f)
					{
						clipped = true;
						break;
					}

					// Homogeneous divide.
					p /= p.w();

					// Track screen space extents.
					mn = min(mn, p);
					mx = max(mx, p);
				}

				const Vector4 e = mx - mn;

				cp.distance = lodDistance;
				cp.area = !clipped ? e.x() * e.y() : Scalar(1000.0f);
				cp.patchId = patchId;
				cp.patchOrigin = patchOrigin;

				m_visiblePatches.push_back(cp);
			}
			else
			{
				m_patches[patchId].lastPatchLod = c_patchLodSteps;
				m_patches[patchId].lastSurfaceLod = c_surfaceLodSteps;

				if (!snapshot)
					m_surfaceCache[viewIndex]->flush(patchId);
			}

			patchOrigin += patchDeltaX;
		}
		patchTopLeft += patchDeltaZ;
	}

	// Sort patches front to back to maximize best use of surface cache and rendering.
	std::sort(m_visiblePatches.begin(), m_visiblePatches.end(), [](const CullPatch& lh, const CullPatch& rh) {
		return lh.distance < rh.distance;
	});

	for (uint32_t i = 0; i < LodCount; ++i)
		m_patchLodInstances[i].resize(0);

	// Update all patch surfaces.
	for (const auto& visiblePatch : m_visiblePatches)
	{
		Patch& patch = m_patches[visiblePatch.patchId];
		const Vector4& patchOrigin = visiblePatch.patchOrigin;

		// Calculate which surface lod to use based one distance to patch center.
		const float distance = max(visiblePatch.distance - detailDistance, 0.0f);
		const float surfaceLodDistance = std::pow(clamp(distance / m_surfaceLodDistance + m_surfaceLodBias, 0.0f, 1.0f), m_surfaceLodExponent);
		const float surfaceLodF = surfaceLodDistance * c_surfaceLodSteps;
		int32_t surfaceLod = int32_t(surfaceLodF + 0.5f);

		const float c_lodHysteresisThreshold = 0.5f;
		if (surfaceLod != patch.lastSurfaceLod)
		{
			if (std::abs(surfaceLodF - patch.lastSurfaceLod) < c_lodHysteresisThreshold)
				surfaceLod = patch.lastSurfaceLod;
		}

		// Find patch lod based on screen space error.
		int32_t patchLod = 0;
		for (int32_t j = 3; j > 0; --j)
		{
			if (visiblePatch.error[j] <= 1.0f)
			{
				patchLod = j;
				break;
			}
		}

		patch.lastPatchLod = patchLod;
		patch.lastSurfaceLod = surfaceLod;
		patch.surfaceOffset = Vector4::zero();

		// Update surface cache.
		if (!snapshot)
		{
			m_surfaceCache[viewIndex]->setupPatch(
		 		context.getRenderGraph(),
		 		m_terrain,
		 		-worldExtent * Scalar(0.5f),
		 		worldExtent,
		 		patchOrigin,
		 		patchExtent,
		 		patch.lastSurfaceLod,
		 		visiblePatch.patchId,
		 		// Out
		 		patch.surfaceOffset
			 );
		}

		// Queue patch instance.
		m_patchLodInstances[patchLod].push_back(&visiblePatch);
	}

	// Update base color texture.
	m_surfaceCache[viewIndex]->setupBaseColor(
		context.getRenderGraph(),
		m_terrain,
		-worldExtent * Scalar(0.5f),
		worldExtent
	);
}

void TerrainComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	float detailDistance,
	uint32_t cacheSize
)
{
	const int32_t viewIndex = worldRenderView.getIndex();
	const bool snapshot = worldRenderView.getSnapshot();

	if (!validate(viewIndex, cacheSize))
		return;

	render::Shader* shader = m_terrain->getTerrainShader();

	auto perm = worldRenderPass.getPermutation(shader);

	shader->setCombination(c_handleTerrain_CutEnable, m_terrain->getCutMap(), perm);
	shader->setCombination(c_handleTerrain_ColorEnable, m_terrain->getColorMap(), perm);

	if (m_visualizeMode >= VmSurfaceLod && m_visualizeMode <= VmPatchLod)
	{
		shader->setCombination(c_handleTerrain_VisualizeLods, true, perm);
	}
	else if (m_visualizeMode >= VmColorMap && m_visualizeMode <= VmMaterialMap)
	{
		shader->setCombination(c_handleTerrain_VisualizeMap, true, perm);
	}

	render::IProgram* program = shader->getProgram(perm).program;
	if (!program)
		return;

	const Vector4& worldExtent = m_heightfield->getWorldExtent();
	const Vector4 eyePosition = worldRenderView.getEyePosition();
	const Vector4 patchExtent(worldExtent.x() / float(m_patchCount), worldExtent.y(), worldExtent.z() / float(m_patchCount), 0.0f);

	render::RenderContext* renderContext = context.getRenderContext();

	// Setup shared shader parameters.
	auto rb = renderContext->alloc< render::NullRenderBlock >(L"Terrain patch setup");

	rb->program = program;
	rb->programParams = renderContext->alloc< render::ProgramParameters >();

	rb->programParams->beginParameters(renderContext);

	rb->programParams->setTextureParameter(c_handleTerrain_Heightfield, m_terrain->getHeightMap());
	
	if (!snapshot)
	{
		rb->programParams->setTextureParameter(c_handleTerrain_SurfaceAlbedo, m_surfaceCache[viewIndex]->getVirtualAlbedo());
		rb->programParams->setTextureParameter(c_handleTerrain_SurfaceNormals, m_surfaceCache[viewIndex]->getVirtualNormals());
	}
	else
	{
		rb->programParams->setTextureParameter(c_handleTerrain_SurfaceAlbedo, m_surfaceCache[viewIndex]->getBaseTexture());
	}

	rb->programParams->setTextureParameter(c_handleTerrain_ColorMap, m_terrain->getColorMap());
	rb->programParams->setTextureParameter(c_handleTerrain_Normals, m_terrain->getNormalMap());
	rb->programParams->setTextureParameter(c_handleTerrain_SplatMap, m_terrain->getSplatMap());
	rb->programParams->setTextureParameter(c_handleTerrain_CutMap, m_terrain->getCutMap());
	rb->programParams->setTextureParameter(c_handleTerrain_MaterialMap, m_terrain->getMaterialMap());
	rb->programParams->setVectorParameter(c_handleTerrain_Eye, eyePosition);
	rb->programParams->setVectorParameter(c_handleTerrain_WorldOrigin, -worldExtent * Scalar(0.5f));
	rb->programParams->setVectorParameter(c_handleTerrain_WorldExtent, worldExtent);

	if (m_visualizeMode == VmColorMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, m_terrain->getColorMap());
	else if (m_visualizeMode == VmNormalMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, m_terrain->getNormalMap());
	else if (m_visualizeMode == VmHeightMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, m_terrain->getHeightMap());
	else if (m_visualizeMode == VmSplatMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, m_terrain->getSplatMap());
	else if (m_visualizeMode == VmCutMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, m_terrain->getCutMap());
	else if (m_visualizeMode == VmMaterialMap)
		rb->programParams->setTextureParameter(c_handleTerrain_DebugMap, m_terrain->getMaterialMap());

	worldRenderPass.setProgramParameters(rb->programParams);

	rb->programParams->endParameters(renderContext);

	renderContext->enqueue(rb);

	// Render each visible patch.
	for (const auto& visiblePatch : m_visiblePatches)
	{
		const Patch& patch = m_patches[visiblePatch.patchId];
		const Vector4& patchOrigin = visiblePatch.patchOrigin;

		auto rb = renderContext->alloc< render::SimpleRenderBlock >(L"Terrain patch");
		rb->distance = visiblePatch.distance;
		rb->program = program;
		rb->programParams = renderContext->alloc< render::ProgramParameters >();
		rb->indexBuffer = m_indexBuffer->getBufferView();
		rb->indexType = render::IndexType::UInt32;
		rb->vertexBuffer = m_vertexBuffer->getBufferView();
		rb->vertexLayout = m_vertexLayout;
		rb->primitives = m_primitives[patch.lastPatchLod];

		rb->programParams->beginParameters(renderContext);

		rb->programParams->setVectorParameter(c_handleTerrain_PatchExtent, patchExtent);
		rb->programParams->setVectorParameter(c_handleTerrain_PatchOrigin, patchOrigin);

		if (!snapshot)
			rb->programParams->setVectorParameter(c_handleTerrain_SurfaceOffset, patch.surfaceOffset);
		else
		{
			rb->programParams->setVectorParameter(c_handleTerrain_SurfaceOffset, Vector4(
				patchOrigin.x() / worldExtent.x() + 0.5f,
				patchOrigin.z() / worldExtent.z() + 0.5f,
				patchExtent.x() / worldExtent.x(),
				patchExtent.z() / worldExtent.z()
			));
		}

		if (m_visualizeMode == VmSurfaceLod)
			rb->programParams->setFloatParameter(c_handleTerrain_DebugPatchIndex, patch.lastSurfaceLod);
		else if (m_visualizeMode == VmPatchLod)
			rb->programParams->setFloatParameter(c_handleTerrain_DebugPatchIndex, patch.lastPatchLod);

		rb->programParams->endParameters(renderContext);

		renderContext->draw(render::RpOpaque, rb);
	}
}

void TerrainComponent::setVisualizeMode(VisualizeMode visualizeMode)
{
	m_visualizeMode = visualizeMode;
}

void TerrainComponent::destroy()
{
}

void TerrainComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void TerrainComponent::setTransform(const Transform& transform)
{
}

Aabb3 TerrainComponent::getBoundingBox() const
{
	const Vector4& worldExtent = m_heightfield->getWorldExtent();
	return Aabb3(-worldExtent, worldExtent);
}

void TerrainComponent::update(const world::UpdateParams& update)
{
}

bool TerrainComponent::validate(int32_t viewIndex, uint32_t cacheSize)
{
	if (
		m_terrain.changed() ||
		m_heightfield.changed()
	)
	{
		m_heightfield.consume();
		m_terrain.consume();

		if (!createPatches())
			return false;
	}

	if (cacheSize != m_cacheSize)
	{
		for (uint32_t i = 0; i < sizeof_array(m_surfaceCache); ++i)
			safeDestroy(m_surfaceCache[i]);
		m_cacheSize = cacheSize;
	}

	if (!m_surfaceCache[viewIndex])
	{
		m_surfaceCache[viewIndex] = new TerrainSurfaceCache();
		if (!m_surfaceCache[viewIndex]->create(m_resourceManager, m_renderSystem, cacheSize))
		{
			m_surfaceCache[viewIndex] = nullptr;
			return false;
		}
	}

	return true;
}

void TerrainComponent::updatePatches(const uint32_t* region)
{
	const uint32_t patchDim = m_terrain->getPatchDim();
	const uint32_t heightfieldSize = m_heightfield->getSize();

	const uint32_t mnx = region ? max< uint32_t >(region[0], 0) : 0;
	const uint32_t mnz = region ? max< uint32_t >(region[1], 0) : 0;
	const uint32_t mxx = region ? min< uint32_t >(region[2] + 1, m_patchCount) : m_patchCount;
	const uint32_t mxz = region ? min< uint32_t >(region[3] + 1, m_patchCount) : m_patchCount;

	for (uint32_t pz = mnz; pz < mxz; ++pz)
	{
		for (uint32_t px = mnx; px < mxx; ++px)
		{
			const uint32_t patchId = px + pz * m_patchCount;

			const int32_t pminX = (heightfieldSize * px) / m_patchCount;
			const int32_t pminZ = (heightfieldSize * pz) / m_patchCount;
			const int32_t pmaxX = (heightfieldSize * (px + 1)) / m_patchCount;
			const int32_t pmaxZ = (heightfieldSize * (pz + 1)) / m_patchCount;

			const Terrain::Patch& patchData = m_terrain->getPatches()[patchId];
			Patch& patch = m_patches[patchId];
			patch.minHeight = patchData.height[0];
			patch.maxHeight = patchData.height[1];
			patch.error[0] = 0.0f;
			patch.error[1] = patchData.error[0];
			patch.error[2] = patchData.error[1];
			patch.error[3] = patchData.error[2];

			for (int32_t i = 0; i < sizeof_array(m_surfaceCache); ++i)
			{
				if (m_surfaceCache[i])
					m_surfaceCache[i]->flush(patchId);
			}
		}
	}
}

bool TerrainComponent::createPatches()
{
	m_patches.clear();
	m_patchCount = 0;
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);

	const uint32_t heightfieldSize = m_heightfield->getSize();
	T_ASSERT(heightfieldSize > 0);

	const uint32_t patchDim = m_terrain->getPatchDim();
	const uint32_t detailSkip = m_terrain->getDetailSkip();

	const uint32_t patchVertexCount = patchDim * patchDim;
	m_patchCount = heightfieldSize / (patchDim * detailSkip);

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, 0));
	const uint32_t vertexSize = render::getVertexSize(vertexElements);

	m_vertexBuffer = m_renderSystem->createBuffer(
		render::BuVertex,
		patchVertexCount,
		vertexSize,
		false
	);
	if (!m_vertexBuffer)
		return false;

	float* vertex = static_cast< float* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (uint32_t z = 0; z < patchDim; ++z)
	{
		for (uint32_t x = 0; x < patchDim; ++x)
		{
			*vertex++ = float(x) / (patchDim - 1);
			*vertex++ = float(z) / (patchDim - 1);
		}
	}

	m_vertexBuffer->unlock();

	m_vertexLayout = m_renderSystem->createVertexLayout(vertexElements);

	m_patches.reserve(m_patchCount * m_patchCount);
	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
			TerrainComponent::Patch patch = { 0.0f, 0.0f, { 0.0f, 0.0f, 0.0f, 0.0f }, c_patchLodSteps, c_surfaceLodSteps };
			m_patches.push_back(patch);
		}
	}

	updatePatches(nullptr);

	AlignedVector< uint32_t > indices;
	for (uint32_t lod = 0; lod < LodCount; ++lod)
	{
		const size_t indexOffset = indices.size();
		const uint32_t lodSkip = 1 << lod;

		for (uint32_t y = 0; y < patchDim - 1; y += lodSkip)
		{
			const uint32_t offset = y * patchDim;
			for (uint32_t x = 0; x < patchDim - 1; x += lodSkip)
			{
				if (lod > 0 && (x == 0 || y == 0 || x == patchDim - 1 - lodSkip || y == patchDim - 1 - lodSkip))
				{
					const int mid = x + offset + (lodSkip >> 1) + (lodSkip >> 1) * patchDim;

					if (x == 0)
					{
						indices.push_back(mid);
						indices.push_back(lodSkip + offset);
						indices.push_back(lodSkip + offset + lodSkip * patchDim);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(offset + i * patchDim + patchDim);
							indices.push_back(offset + i * patchDim);
						}
					}
					else if (x == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i * patchDim + lodSkip);
							indices.push_back(x + offset + i * patchDim + lodSkip + patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip);
						indices.push_back(x + offset + lodSkip + lodSkip * patchDim);
					}

					if (y == 0)
					{
						indices.push_back(mid);
						indices.push_back(x + lodSkip * patchDim + offset + lodSkip);
						indices.push_back(x + lodSkip * patchDim + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i);
							indices.push_back(x + offset + i + 1);
						}
					}
					else if (y == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i + lodSkip * patchDim + 1);
							indices.push_back(x + offset + i + lodSkip * patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim + lodSkip);
						indices.push_back(x + offset + lodSkip * patchDim);
					}
				}
				else
				{
					indices.push_back(x + offset);
					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);

					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);
				}
			}
		}

		const size_t indexEndOffset = indices.size();
		T_ASSERT((indexEndOffset - indexOffset) % 3 == 0);

		const uint32_t minIndex = *std::min_element(indices.begin() + indexOffset, indices.begin() + indexEndOffset);
		const uint32_t maxIndex = *std::max_element(indices.begin() + indexOffset, indices.begin() + indexEndOffset);

		T_ASSERT(minIndex < patchVertexCount);
		T_ASSERT(maxIndex < patchVertexCount);

		m_primitives[lod].setIndexed(
			render::PrimitiveType::Triangles,
			(uint32_t)indexOffset,
			(uint32_t)(indexEndOffset - indexOffset) / 3,
			minIndex,
			maxIndex
		);
	}

	m_indexBuffer = m_renderSystem->createBuffer(
		render::BuIndex,
		(uint32_t)indices.size(),
		sizeof(uint32_t),
		false
	);
	if (!m_indexBuffer)
		return false;

	uint32_t* index = static_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < uint32_t(indices.size()); ++i)
		index[i] = indices[i];

	m_indexBuffer->unlock();
	return true;
}

}
