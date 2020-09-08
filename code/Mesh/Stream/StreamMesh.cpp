#include <algorithm>
#include "Core/Io/IStream.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Mesh/MeshReader.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

struct NamedMeshPart
{
	std::wstring m_meshPart;

	NamedMeshPart(const std::wstring& meshPart)
	:	m_meshPart(meshPart)
	{
	}

	bool operator () (const render::Mesh::Part& meshPart) const
	{
		return meshPart.name == m_meshPart;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StreamMesh", StreamMesh, IMesh)

StreamMesh::StreamMesh()
{
}

const Aabb3& StreamMesh::getBoundingBox() const
{
	return m_boundingBox;
}

bool StreamMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

uint32_t StreamMesh::getFrameCount() const
{
	return (uint32_t)m_frameOffsets.size();
}

Ref< StreamMesh::Instance > StreamMesh::createInstance() const
{
	Ref< Instance > instance = new Instance();
	instance->frame = ~0U;
	return instance;
}

void StreamMesh::build(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	Instance* instance,
	uint32_t frame,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	// Load mesh frame if different from instance's cached frame.
	if (instance->frame != frame || !instance->mesh[0] || !instance->mesh[1])
	{
		m_stream->seek(IStream::SeekSet, m_frameOffsets[frame]);
		instance->mesh[1] = instance->mesh[0];
		instance->mesh[0] = m_meshReader->read(m_stream);
		instance->frame = frame;
	}

	if (!instance->mesh[0])
		return;

	auto it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT(it != m_parts.end());

	const AlignedVector< render::Mesh::Part >& meshParts = instance->mesh[0]->getParts();
	for (const auto part : it->second)
	{
		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		// \fixme Linear search by string
		auto j = std::find_if(meshParts.begin(), meshParts.end(), NamedMeshPart(part.meshPart));
		if (j == meshParts.end())
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"StreamMesh");
		renderBlock->distance = distance;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = instance->mesh[0]->getIndexBuffer();
		renderBlock->vertexBuffer = instance->mesh[0]->getVertexBuffer();
		renderBlock->primitives = j->primitives;

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
			sp.priority,
			renderBlock
		);
	}
}

	}
}
