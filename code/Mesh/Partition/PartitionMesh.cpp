#include "Core/Log/Log.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Partition/IPartition.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.PartitionMesh", PartitionMesh, IMesh)

const Aabb3& PartitionMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool PartitionMesh::supportTechnique(render::handle_t technique) const
{
	return true;
}

void PartitionMesh::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	if (!m_partition)
		return;

	Matrix44 worldView = worldRenderView.getView() * worldTransform.toMatrix44();

	m_partIndices.resize(0);
	m_partition->traverse(
		worldRenderView.getCullFrustum(),
		worldView,
		worldRenderPass.getTechnique(),
		m_partIndices
	);
	if (m_partIndices.empty())
		return;

	const AlignedVector< render::Mesh::Part >& meshParts = m_mesh->getParts();
	for (auto partIndex : m_partIndices)
	{
		const Part& part = m_parts[partIndex];

		m_shader->setTechnique(part.shaderTechnique);
		worldRenderPass.setShaderCombination(m_shader);

		render::IProgram* program = m_shader->getCurrentProgram();
		if (!program)
			continue;

		Vector4 center = worldView * part.boundingBox.getCenter();
		Scalar distancePart = center.z() + part.boundingBox.getExtent().length();

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"PartitionMesh");
		renderBlock->distance = distancePart;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			worldTransform,
			worldTransform,
			getBoundingBox()
		);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			m_shader->getCurrentPriority(),
			renderBlock
		);
	}
}

	}
}
