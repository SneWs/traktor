/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Resource/IResourceManager.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/UndergrowthComponent.h"
#include "Terrain/UndergrowthComponentData.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::terrain
{
	namespace
	{

const int32_t c_maxInstanceCount = 180;

#pragma pack(1)
struct Vertex
{
	float position[2];
	half_t texCoord[2];
};
#pragma pack()

const render::Handle s_handleTerrain_Normals(L"Terrain_Normals");
const render::Handle s_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle s_handleTerrain_SurfaceAlbedo(L"Terrain_SurfaceAlbedo");
const render::Handle s_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle s_handleUndergrowth_Eye(L"Undergrowth_Eye");
const render::Handle s_handleUndergrowth_MaxDistance(L"Undergrowth_MaxDistance");
const render::Handle s_handleUndergrowth_Instances1(L"Undergrowth_Instances1");
const render::Handle s_handleUndergrowth_Instances2(L"Undergrowth_Instances2");

Vertex packVertex(const Vector4& position, float u, float v)
{
	Vertex vtx;
	vtx.position[0] = position.x();
	vtx.position[1] = position.y();
	vtx.texCoord[0] = floatToHalf(u);
	vtx.texCoord[1] = floatToHalf(v);
	return vtx;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.UndergrowthComponent", UndergrowthComponent, TerrainLayerComponent)

bool UndergrowthComponent::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const UndergrowthComponentData& layerData
)
{
	m_layerData = layerData;

	if (!resourceManager->bind(m_layerData.m_shader, m_shader))
		return false;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtHalf2, offsetof(Vertex, texCoord)));
	T_ASSERT(render::getVertexSize(vertexElements) == sizeof(Vertex));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(
		render::BuVertex,
		4,
		sizeof(Vertex),
		false
	);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	const Vector4 position(0.0f, 0.0f, 0.0f);
	const Vector4 axisX(1.0f, 0.0f, 0.0f);
	const Vector4 axisY(0.0f, 1.0f, 0.0f);

	*vertex++ = packVertex(position - axisX - axisY, 0.0f, 1.0f);
	*vertex++ = packVertex(position - axisX + axisY, 0.0f, 0.0f);
	*vertex++ = packVertex(position + axisX + axisY, 1.0f, 0.0f);
	*vertex++ = packVertex(position + axisX - axisY, 1.0f, 1.0f);

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createBuffer(
		render::BuIndex,
		3 * 2 * 2,
		sizeof(uint16_t),
		false
	);
	if (!m_indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());

	*index++ = 0;
	*index++ = 1;
	*index++ = 2;

	*index++ = 0;
	*index++ = 2;
	*index++ = 3;

	*index++ = 2;
	*index++ = 1;
	*index++ = 0;

	*index++ = 3;
	*index++ = 2;
	*index++ = 0;

	m_indexBuffer->unlock();
	return true;
}

void UndergrowthComponent::destroy()
{
}

void UndergrowthComponent::setOwner(world::Entity* owner)
{
	TerrainLayerComponent::setOwner(owner);
	m_owner = owner;
}

void UndergrowthComponent::setTransform(const Transform& transform)
{
}

Aabb3 UndergrowthComponent::getBoundingBox() const
{
	return Aabb3();
}

void UndergrowthComponent::update(const world::UpdateParams& update)
{
	TerrainLayerComponent::update(update);
}

void UndergrowthComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const auto& terrain = terrainComponent->getTerrain();

	// Update clusters at first pass from eye pow.
	bool updateClusters = (bool)((worldRenderPass.getPassFlags() & world::IWorldRenderPass::First) != 0);

	const Matrix44 view = worldRenderView.getView();
	const Matrix44 viewInv = view.inverse();
	const Vector4 eye = viewInv.translation();

	// Get plant state for current view.
	ViewState& vs = m_viewState[worldRenderView.getIndex()];
	if (vs.plants.size() != m_plantsCount * 2)
	{
		vs.plants.resize(m_plantsCount * 2, Vector4::zero());
		vs.distances.resize(m_clusters.size(), 0.0f);
		vs.pvs.assign((uint32_t)m_clusters.size(), false);
		updateClusters = true;
	}

	if (updateClusters)
	{
		Frustum viewFrustum = worldRenderView.getViewFrustum();
		viewFrustum.setFarZ(Scalar(m_layerData.m_spreadDistance + m_clusterSize));

		// Only perform "replanting" half of clusters each frame.
		const Scalar clusterSize(m_clusterSize);
		for (uint32_t i = vs.count % 2; i < m_clusters.size(); i += 2)
		{
			const Cluster& cluster = m_clusters[i];

			vs.distances[i] = (cluster.center - eye).length();

			const bool visible = vs.pvs[i];
			vs.pvs.set(i, viewFrustum.inside(view * cluster.center, clusterSize) != Frustum::Result::Outside);
			if (!vs.pvs[i])
				continue;
			if (vs.pvs[i] && visible)
				continue;

			RandomGeometry random(int32_t(cluster.center.x() * 919.0f + cluster.center.z() * 463.0f));
			for (int32_t j = cluster.from; j < cluster.to; ++j)
			{
				const Vector2 ruv = Quasirandom::hammersley(j - cluster.from, cluster.to - cluster.from, random);

				const float dx = (ruv.x * 2.2f - 1.1f) * m_clusterSize;
				const float dz = (ruv.y * 2.2f - 1.1f) * m_clusterSize;

				const float px = cluster.center.x() + dx;
				const float pz = cluster.center.z() + dz;

				vs.plants[j * 2 + 0] = Vector4(
					px,
					pz,
					float(cluster.plant),
					0.0f
				);
				vs.plants[j * 2 + 1] = Vector4(
					cluster.plantScale * (random.nextFloat() * 0.5f + 0.5f),
					random.nextFloat(),
					0.0f,
					0.0f
				);
			}
		}

		vs.count++;
	}

	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;

	render::RenderContext* renderContext = context.getRenderContext();

	render::ProgramParameters* extraParameters = renderContext->alloc< render::ProgramParameters >();
	extraParameters->beginParameters(renderContext);
	extraParameters->setTextureParameter(s_handleTerrain_Normals, terrain->getNormalMap());
	extraParameters->setTextureParameter(s_handleTerrain_Heightfield, terrain->getHeightMap());
	extraParameters->setTextureParameter(s_handleTerrain_SurfaceAlbedo, terrainComponent->getSurfaceCache(worldRenderView.getIndex())->getBaseTexture());
	extraParameters->setVectorParameter(s_handleTerrain_WorldExtent, terrain->getHeightfield()->getWorldExtent());
	extraParameters->setVectorParameter(s_handleUndergrowth_Eye, eye);
	extraParameters->setFloatParameter(s_handleUndergrowth_MaxDistance, m_layerData.m_spreadDistance + m_clusterSize);
	extraParameters->endParameters(renderContext);

	Vector4 instanceData1[c_maxInstanceCount];
	Vector4 instanceData2[c_maxInstanceCount];

	for (uint32_t i = 0; i < m_clusters.size(); ++i)
	{
		if (!vs.pvs[i])
			continue;

		const Cluster& cluster = m_clusters[i];

		const int32_t count = cluster.to - cluster.from;
		for (int32_t j = 0; j < count; )
		{
			const int32_t batch = std::min(count - j, c_maxInstanceCount);

			for (int32_t k = 0; k < batch; ++k, ++j)
			{
				instanceData1[k] = vs.plants[(j + cluster.from) * 2 + 0];
				instanceData2[k] = vs.plants[(j + cluster.from) * 2 + 1];
			}

			auto renderBlock = renderContext->alloc< render::IndexedInstancingRenderBlock >(L"Undergrowth");

			renderBlock->distance = vs.distances[i];
			renderBlock->program = sp.program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_indexBuffer->getBufferView();
			renderBlock->indexType = render::IndexType::UInt16;
			renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
			renderBlock->vertexLayout = m_vertexLayout;
			renderBlock->primitive = render::PrimitiveType::Triangles;
			renderBlock->offset = 0;
			renderBlock->count = 2 * 2;
			renderBlock->minIndex = 0;
			renderBlock->maxIndex = 3;
			renderBlock->instanceCount = batch;

			renderBlock->programParams->beginParameters(renderContext);

			if (extraParameters)
				renderBlock->programParams->attachParameters(extraParameters);

			worldRenderPass.setProgramParameters(renderBlock->programParams);
			renderBlock->programParams->setVectorArrayParameter(s_handleUndergrowth_Instances1, instanceData1, batch);
			renderBlock->programParams->setVectorArrayParameter(s_handleUndergrowth_Instances2, instanceData2, batch);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(
				sp.priority,
				renderBlock
			);
		}
	}
}

void UndergrowthComponent::updatePatches()
{
	m_clusters.resize(0);
	m_plantsCount = 0;

	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const resource::Proxy< Terrain >& terrain = terrainComponent->getTerrain();
	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();

	// Get set of materials which have undergrowth.
	StaticVector< uint8_t, 16 > um;
	um.resize(16, 0);

	uint8_t maxMaterialIndex = 0;
	for (const auto& plant : m_layerData.m_plants)
		um[plant.attribute] = ++maxMaterialIndex;

	const int32_t size = heightfield->getSize();
	const Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(size));

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	// Create clusters.
	RandomGeometry random;
	for (int32_t z = 0; z < size; z += 16)
	{
		for (int32_t x = 0; x < size; x += 16)
		{
			StaticVector< int32_t, 16 > cm;
			cm.resize(16, 0);

			int32_t totalDensity = 0;
			for (int32_t cz = 0; cz < 16; ++cz)
			{
				for (int32_t cx = 0; cx < 16; ++cx)
				{
					const uint8_t attribute = heightfield->getGridAttribute(x + cx, z + cz);
					const uint8_t index = um[attribute];
					if (index > 0)
					{
						cm[index - 1]++;
						totalDensity++;
					}
				}
			}
			if (totalDensity <= 0)
				continue;

			float wx, wz;
			heightfield->gridToWorld(x + 8, z + 8, wx, wz);

			const float wy = heightfield->getWorldHeight(wx, wz);

			for (uint32_t i = 0; i < maxMaterialIndex; ++i)
			{
				if (cm[i] <= 0)
					continue;

				for (const auto& plant : m_layerData.m_plants)
				{
					if (um[plant.attribute] == i + 1)
					{
						const int32_t densityFactor = cm[i];

						const int32_t density = (plant.density * densityFactor) / (16 * 16);
						if (density <= 4)
							continue;

						const int32_t from = m_plantsCount;
						const int32_t to = from + density;

						Cluster c;
						c.center = Vector4(wx, wy, wz, 1.0f);
						c.plant = plant.plant;
						c.plantScale = plant.scale * (0.5f + 0.5f * densityFactor / (16.0f * 16.0f));
						c.from = from;
						c.to = to;
						m_clusters.push_back(c);

						m_plantsCount = to;
					}
				}
			}
		}
	}
}

}
