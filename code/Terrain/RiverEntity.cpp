#include "Core/Math/Hermite.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/RiverEntity.h"
#include "Terrain/RiverEntityData.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

struct ControlPointAccessor
{
	typedef float time_t;
	typedef RiverEntityData::ControlPoint key_t;

	static time_t time(const key_t* keys, size_t nkeys, const key_t& key)
	{
		int32_t index = int32_t(&key - keys);
		return index / time_t(nkeys - 1);
	}

	static const key_t& value(const key_t& key)
	{
		return key;
	}

	static key_t combine(
		const key_t& v0, const time_t& w0,
		const key_t& v1, const time_t& w1,
		const key_t& v2, const time_t& w2,
		const key_t& v3, const time_t& w3
	)
	{
		key_t c;
		c.width = v0.width * w0 + v1.width * w1 + v2.width * w2 + v3.width * w3;
		c.position = v0.position * Scalar(w0) + v1.position * Scalar(w1) + v2.position * Scalar(w2) + v3.position * Scalar(w3);
		c.tension = v0.tension * w0 + v1.tension * w1 + v2.tension * w2 + v3.tension * w3;
		return c;
	}
};

typedef Hermite< RiverEntityData::ControlPoint, float, RiverEntityData::ControlPoint, ControlPointAccessor > hermite_t;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.RiverEntity", RiverEntity, world::Entity)

RiverEntity::RiverEntity()
{
}

bool RiverEntity::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const RiverEntityData& data)
{
	const AlignedVector< RiverEntityData::ControlPoint >& path = data.getPath();
	if (path.size() < 2)
		return false;

	float length = 0.0f;
	for (AlignedVector< RiverEntityData::ControlPoint >::const_iterator i = path.begin() + 1; i != path.end(); ++i)
		length += (i->position - (i - 1)->position).length();

	float dT = std::max(1.0f / length, 0.001f);

	AlignedVector< Vector4 > silouette;

	hermite_t h(path.c_ptr(), path.size());
	RiverEntityData::ControlPoint pc = h.evaluate(0.0f);
	for (float T = dT; T <= 1.0f; )
	{
		RiverEntityData::ControlPoint pn = h.evaluate(T);

		Vector4 d = pn.position - pc.position;
		Vector4 e = cross(d, Vector4(0.0f, 1.0f, 0.0f, 0.0f)).normalized();

		silouette.push_back((pc.position - e * Scalar(pc.width / 2.0f)).xyz0() + Vector4(0.0f, 0.0f, 0.0f, T));
		silouette.push_back((pc.position + e * Scalar(pc.width / 2.0f)).xyz0() + Vector4(0.0f, 0.0f, 0.0f, T));

		pc = pn;

		T += dT * pn.tension;
	}

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat4, 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, 4 * sizeof(float)));

	m_vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		silouette.size() * sizeof(float) * 6,
		false
	);
	if (!m_vertexBuffer)
		return false;

	float* vertex = static_cast< float* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	float u = 0.0f;
	for (AlignedVector< Vector4 >::const_iterator i = silouette.begin(); i != silouette.end(); ++i)
	{
		i->xyz1().storeUnaligned(vertex);
		vertex += 4;

		*vertex++ = u;
		*vertex++ = data.getTileFactorV() * i->w();

		u = 1.0f - u;
	}

	m_vertexBuffer->unlock();

	uint32_t triangleCount = silouette.size() - 2;

	m_indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		triangleCount * 3 * sizeof(uint16_t),
		false
	);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	if (!index)
		return false;

	for (uint16_t i = 0; i < uint16_t(triangleCount); i += 2)
	{
		*index++ = i + 0;
		*index++ = i + 1;
		*index++ = i + 2;
		*index++ = i + 3;
		*index++ = i + 1;
		*index++ = i + 2;
	}

	m_indexBuffer->unlock();

	m_primitives.setIndexed(
		render::PtTriangles,
		0,
		triangleCount,
		0,
		silouette.size() - 1
	);

	m_shader = data.getShader();
	if (!resourceManager->bind(m_shader))
		return false;

	return true;
}

void RiverEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_shader.validate())
		return;

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("River");

	renderBlock->distance = 0.0f;
	renderBlock->program = program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = &m_primitives;

	renderBlock->programParams->beginParameters(renderContext);
	
	worldRenderPass.setProgramParameters(renderBlock->programParams);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(render::RfAlphaBlend, renderBlock);
}

Aabb3 RiverEntity::getBoundingBox() const
{
	return Aabb3();
}

void RiverEntity::update(const world::EntityUpdate* update)
{
}

	}
}
