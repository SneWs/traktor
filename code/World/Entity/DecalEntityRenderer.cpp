#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/DecalEntity.h"
#include "World/Entity/DecalEntityRenderer.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

struct Vertex
{
	float position[3];
};

render::handle_t s_handleDecalSize;
render::handle_t s_handleDecalThickness;
render::handle_t s_handleAlpha;
render::handle_t s_handleAge;
render::handle_t s_handleMagicCoeffs;
render::handle_t s_handleWorldViewInv;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalEntityRenderer", DecalEntityRenderer, IEntityRenderer)

DecalEntityRenderer::DecalEntityRenderer(render::IRenderSystem* renderSystem)
{
	s_handleDecalSize = render::getParameterHandle(L"DecalSize");
	s_handleDecalThickness = render::getParameterHandle(L"DecalThinkness");
	s_handleAlpha = render::getParameterHandle(L"Alpha");
	s_handleAge = render::getParameterHandle(L"Age");
	s_handleMagicCoeffs = render::getParameterHandle(L"MagicCoeffs");
	s_handleWorldViewInv = render::getParameterHandle(L"WorldViewInv");

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(Vertex, position), 0));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(Vertex), L"Incorrect size of vertex");

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 8 * sizeof(Vertex), false);
	T_ASSERT_M (m_vertexBuffer, L"Unable to create vertex buffer");

	Vector4 extents[8];
	Aabb3(Vector4(-1.0f, -1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)).getExtents(extents);

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	T_ASSERT (vertex);

	for (uint32_t i = 0; i < sizeof_array(extents); ++i)
	{
		vertex->position[0] = extents[i].x();
		vertex->position[1] = extents[i].y();
		vertex->position[2] = extents[i].z();
		vertex++;
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, 6 * 2 * 3 * sizeof(uint16_t), false);
	T_ASSERT_M (m_indexBuffer, L"Unable to create index buffer");

	const int32_t* faces = Aabb3::getFaces();

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	for (uint32_t i = 0; i < 6; ++i)
	{
		*index++ = faces[i * 4 + 0];
		*index++ = faces[i * 4 + 1];
		*index++ = faces[i * 4 + 3];

		*index++ = faces[i * 4 + 1];
		*index++ = faces[i * 4 + 2];
		*index++ = faces[i * 4 + 3];
	}

	m_indexBuffer->unlock();
}

const TypeInfoSet DecalEntityRenderer::getEntityTypes() const
{
	TypeInfoSet TypeInfoSet;
	TypeInfoSet.insert(&type_of< DecalEntity >());
	return TypeInfoSet;
}

void DecalEntityRenderer::precull(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	Entity* entity
)
{
}

void DecalEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Entity* entity
)
{
	DecalEntity* decalEntity = checked_type_cast< DecalEntity*, false >(entity);

	Transform transform;
	decalEntity->getTransform(transform);

	float s = decalEntity->getSize();
	float t = decalEntity->getThickness();

	Vector4 center = worldRenderView.getView() * transform.translation().xyz1();
	Scalar radius = Scalar(std::sqrt(s * s + s * s + t * t));

	if (worldRenderView.getCullFrustum().inside(center, radius) == Frustum::IrOutside)
		return;

	m_decalEntities.push_back(decalEntity);
}

void DecalEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
	render::RenderContext* renderContext = worldContext.getRenderContext();
	T_ASSERT (renderContext);

	const Matrix44& projection = worldRenderView.getProjection();
	const Matrix44& view = worldRenderView.getView();

	const Scalar p11 = projection.get(0, 0);
	const Scalar p22 = projection.get(1, 1);

	// Render all decal boxes.
	for (RefArray< DecalEntity >::iterator i = m_decalEntities.begin(); i != m_decalEntities.end(); ++i)
	{
		Transform transform;
		(*i)->getTransform(transform);

		render::Shader* shader = (*i)->getShader();
		T_ASSERT (shader);

		worldRenderPass.setShaderTechnique(shader);
		worldRenderPass.setShaderCombination(shader);

		render::IProgram* program = shader->getCurrentProgram();
		if (!program)
			continue;

		Matrix44 worldView = view * transform.toMatrix44();
		Matrix44 worldViewInv = worldView.inverse();

		render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("Decal");

		renderBlock->distance = 0.0f;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_indexBuffer;
		renderBlock->vertexBuffer = m_vertexBuffer;
		renderBlock->primitive = render::PtTriangles;
		renderBlock->offset = 0;
		renderBlock->count = 12;
		renderBlock->minIndex = 0;
		renderBlock->maxIndex = 7;

		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			true,
			transform.toMatrix44(),
			(*i)->getBoundingBox()
		);

		renderBlock->programParams->setFloatParameter(s_handleDecalSize, (*i)->getSize());
		renderBlock->programParams->setFloatParameter(s_handleDecalThickness, (*i)->getThickness());
		renderBlock->programParams->setFloatParameter(s_handleAlpha, (*i)->getAlpha());
		renderBlock->programParams->setFloatParameter(s_handleAge, (*i)->getAge());
		renderBlock->programParams->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		renderBlock->programParams->setMatrixParameter(s_handleWorldViewInv, worldViewInv);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(render::RfPostOpaque, renderBlock);
	}

	// Flush all queued decals.
	m_decalEntities.resize(0);
}

	}
}
