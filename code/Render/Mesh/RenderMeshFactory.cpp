#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderMeshFactory", RenderMeshFactory, MeshFactory)

RenderMeshFactory::RenderMeshFactory(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

Ref< Mesh > RenderMeshFactory::createMesh(
	const AlignedVector< VertexElement >& vertexElements,
	uint32_t vertexBufferSize,
	IndexType indexType,
	uint32_t indexBufferSize
) const
{
	Ref< const IVertexLayout > vertexLayout;
	Ref< Buffer > vertexBuffer;
	Ref< Buffer > indexBuffer;

	if (vertexBufferSize > 0)
	{
		vertexLayout = m_renderSystem->createVertexLayout(vertexElements);
		if (!vertexLayout)
			return nullptr;

		vertexBuffer = m_renderSystem->createBuffer(BuVertex, vertexBufferSize, false);
		if (!vertexBuffer)
			return nullptr;
	}

	if (indexBufferSize > 0)
	{
		indexBuffer = m_renderSystem->createBuffer(BuIndex, indexBufferSize, false);
		if (!indexBuffer)
			return nullptr;
	}

	Ref< Mesh > mesh = new Mesh();
	mesh->setVertexElements(vertexElements);
	mesh->setVertexLayout(vertexLayout);
	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexType(indexType);
	mesh->setIndexBuffer(indexBuffer);
	return mesh;
}

	}
}
