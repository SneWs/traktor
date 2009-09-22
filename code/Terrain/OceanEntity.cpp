#include "Terrain/OceanEntity.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/Heightfield.h"
#include "World/WorldRenderView.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanEntity", OceanEntity, world::Entity)

OceanEntity::OceanEntity()
:	m_altitude(0.0f)
{
}

bool OceanEntity::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanEntityData& data)
{
	const uint32_t gridSize = 100;
	const uint32_t vertexCount = gridSize * gridSize;
	const uint32_t triangleCount = (gridSize - 1) * (gridSize - 1) * 2;
	const uint32_t indexCount = triangleCount * 3;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, 0));

	m_vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		vertexCount * sizeof(float) * 2,
		false
	);
	if (!m_vertexBuffer)
		return 0;

	float* vertex = static_cast< float* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (int y = 0; y < gridSize; ++y)
	{
		float fy = 2.0f * float(y) / (gridSize - 1) - 1.0f;
		for (int x = 0; x < gridSize; ++x)
		{
			float fx = 2.0f * float(x) / (gridSize - 1) - 1.0f;
			*vertex++ = fx;
			*vertex++ = fy;
		}
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt32,
		indexCount * sizeof(uint32_t),
		false
	);
	if (!m_indexBuffer)
		return 0;

	uint32_t* index = static_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (int i = 0; i < gridSize - 1; ++i)
	{
		for (int j = 0; j < gridSize - 1; ++j)
		{
			#define IDX(ii, jj) uint32_t((ii) + (jj) * gridSize)

			*index++ = IDX(i + 1, j + 1);
			*index++ = IDX(i    , j + 1);
			*index++ = IDX(i    , j    );
			*index++ = IDX(i + 1, j    );
			*index++ = IDX(i + 1, j + 1);
			*index++ = IDX(i    , j    );
		}
	}

	m_indexBuffer->unlock();

	m_primitives.setIndexed(
		render::PtTriangles,
		0,
		triangleCount,
		0,
		indexCount - 1
	);

	m_screenRenderer = gc_new< render::ScreenRenderer >();
	if (!m_screenRenderer->create(renderSystem))
		return 0;

	m_heightfield = data.m_heightfield;
	m_shader = data.m_shader;
	m_altitude = data.m_altitude;

	if (!resourceManager->bind(m_heightfield))
		return false;
	if (!resourceManager->bind(m_shader))
		return false;

	T_ASSERT (MaxWaves <= OceanEntityData::MaxWaves);
	for (int i = 0; i < MaxWaves; ++i)
		m_waveData[i] = Vector4(data.m_waves[i].direction.x, data.m_waves[i].direction.y, data.m_waves[i].amplitude, data.m_waves[i].phase);

	return true;
}

void OceanEntity::render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView)
{
	if (!m_shader.validate())
		return;

	if (!m_shader->hasTechnique(worldRenderView->getTechnique()))
		return;

	Matrix44 viewInverse = worldRenderView->getView().inverse();

	Vector4 cameraPosition = worldRenderView->getEyePosition();
	Matrix44 oceanWorld = translate(-cameraPosition.x(), -m_altitude, -cameraPosition.z());

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

	renderBlock->type = render::RbtAlphaBlend;
	renderBlock->distance = 0.0f;
	renderBlock->shader = m_shader;
	renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = &m_primitives;

	renderBlock->shaderParams->beginParameters(renderContext);
	
	worldRenderView->setShaderParameters(
		renderBlock->shaderParams,
		oceanWorld,
		oceanWorld,	// @fixme
		Aabb()
	);

	renderBlock->shaderParams->setFloatParameter(L"ViewPlane", worldRenderView->getViewFrustum().getNearZ());
	renderBlock->shaderParams->setFloatParameter(L"OceanRadius", worldRenderView->getViewFrustum().getFarZ());
	renderBlock->shaderParams->setFloatParameter(L"OceanAltitude", m_altitude);
	renderBlock->shaderParams->setVectorParameter(L"CameraPosition", cameraPosition);
	renderBlock->shaderParams->setVectorArrayParameter(L"WaveData", m_waveData, MaxWaves);
	renderBlock->shaderParams->setMatrixParameter(L"ViewInverse", viewInverse);
	renderBlock->shaderParams->setFloatParameter(L"ViewRatio", worldRenderView->getViewSize().x / worldRenderView->getViewSize().y);

	if (m_heightfield.validate())
	{
		renderBlock->shaderParams->setVectorParameter(L"WorldOrigin", -(m_heightfield->getResource().getWorldExtent() * Scalar(0.5f)).xyz1());
		renderBlock->shaderParams->setVectorParameter(L"WorldExtent", m_heightfield->getResource().getWorldExtent().xyz0());
		renderBlock->shaderParams->setSamplerTexture(L"Heightfield", m_heightfield->getHeightTexture());
	}

	renderBlock->shaderParams->endParameters(renderContext);

	renderContext->draw(renderBlock);
}

void OceanEntity::update(const world::EntityUpdate* update)
{
}

	}
}
