#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMesh", StaticMesh, IMesh)

const Aabb3& StaticMesh::getBoundingBox() const
{
	return m_renderMesh->getBoundingBox();
}

// bool StaticMesh::supportTechnique(render::handle_t technique) const
// {
// 	return m_parts.find(technique) != m_parts.end();
// }

const AlignedVector< StaticMesh::Part >* StaticMesh::findParts(render::handle_t technique) const
{
	auto it = m_parts.find(technique);
	return it != m_parts.end() ? &it->second : nullptr;
}

void StaticMesh::build(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const parts_t* parts,
	const Transform& lastWorldTransform,
	const Transform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	const auto& meshParts = m_renderMesh->getParts();
	for (const auto& part : *parts)
	{
		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;		

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"StaticMesh");
		renderBlock->distance = distance;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			lastWorldTransform,
			worldTransform
		);

		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			sp.priority,
			renderBlock
		);
	}
}

	}
}
