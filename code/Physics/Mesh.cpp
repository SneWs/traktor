#include "Physics/Mesh.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const uint32_t c_version = 5;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Mesh", Mesh, Object)

Mesh::Mesh()
:	m_margin(0.0f)
{
}

void Mesh::setVertices(const AlignedVector< Vector4 >& vertices)
{
	m_vertices = vertices;
}

const AlignedVector< Vector4 >& Mesh::getVertices() const
{
	return m_vertices;
}

void Mesh::setShapeTriangles(const AlignedVector< Triangle >& shapeTriangles)
{
	m_shapeTriangles = shapeTriangles;
}

const AlignedVector< Mesh::Triangle >& Mesh::getShapeTriangles() const
{
	return m_shapeTriangles;
}

void Mesh::setHullTriangles(const AlignedVector< Triangle >& hullTriangles)
{
	m_hullTriangles = hullTriangles;
}

const AlignedVector< Mesh::Triangle >& Mesh::getHullTriangles() const
{
	return m_hullTriangles;
}

void Mesh::setHullIndices(const AlignedVector< uint32_t >& hullIndices)
{
	m_hullIndices = hullIndices;
}

const AlignedVector< uint32_t >& Mesh::getHullIndices() const
{
	return m_hullIndices;
}

void Mesh::setOffset(const Vector4& offset)
{
	m_offset = offset;
}

const Vector4& Mesh::getOffset() const
{
	return m_offset;
}

void Mesh::setMargin(float margin)
{
	m_margin = margin;
}

float Mesh::getMargin() const
{
	return m_margin;
}

bool Mesh::read(IStream* stream)
{
	Reader rd(stream);

	uint32_t version;
	rd >> version;

	if (version != c_version)
		return false;

	uint32_t vertexCount;
	rd >> vertexCount;

	uint32_t shapeTriangleCount;
	rd >> shapeTriangleCount;

	uint32_t hullTriangleCount;
	rd >> hullTriangleCount;

	uint32_t hullIndexCount;
	rd >> hullIndexCount;

	m_vertices.resize(vertexCount);
	if (vertexCount > 0)
		rd.read(&m_vertices[0], vertexCount, sizeof(Vector4));

	m_shapeTriangles.resize(shapeTriangleCount);
	if (shapeTriangleCount > 0)
		rd.read(&m_shapeTriangles[0], shapeTriangleCount, sizeof(Triangle));

	m_hullTriangles.resize(hullTriangleCount);
	if (hullTriangleCount > 0)
		rd.read(&m_hullTriangles[0], hullTriangleCount, sizeof(Triangle));

	m_hullIndices.resize(hullIndexCount);
	if (hullIndexCount > 0)
		rd.read(&m_hullIndices[0], hullIndexCount, sizeof(uint32_t));

	float offset[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	rd >> offset[0];
	rd >> offset[1];
	rd >> offset[2];
	m_offset = Vector4::loadUnaligned(offset);

	rd >> m_margin;

	return true;
}

bool Mesh::write(IStream* stream)
{
	Writer wr(stream);

	wr << uint32_t(c_version);
	wr << uint32_t(m_vertices.size());
	wr << uint32_t(m_shapeTriangles.size());
	wr << uint32_t(m_hullTriangles.size());
	wr << uint32_t(m_hullIndices.size());

	if (!m_vertices.empty())
		wr.write(&m_vertices[0], int(m_vertices.size()), sizeof(Vector4));

	if (!m_shapeTriangles.empty())
		wr.write(&m_shapeTriangles[0], int(m_shapeTriangles.size()), sizeof(Triangle));

	if (!m_hullTriangles.empty())
		wr.write(&m_hullTriangles[0], int(m_hullTriangles.size()), sizeof(Triangle));

	if (!m_hullIndices.empty())
		wr.write(&m_hullIndices[0], int(m_hullIndices.size()), sizeof(uint32_t));

	float offset[4];
	m_offset.storeUnaligned(offset);
	wr << offset[0];
	wr << offset[1];
	wr << offset[2];

	wr << m_margin;

	return true;
}

	}
}
