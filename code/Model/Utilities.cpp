#include <algorithm>
#include <numeric>
#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Triangle.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding3.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/ContainerHelpers.h"
#include "Model/Model.h"
#include "Model/TriangleOrderForsyth.h"
#include "Model/Utilities.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct TangentBase
{
	Vector4 normal;
	Vector4 tangent;
	Vector4 binormal;

	TangentBase()
	:	normal(0.0f, 0.0f, 0.0f, 0.0f)
	,	tangent(0.0f, 0.0f, 0.0f, 0.0f)
	,	binormal(0.0f, 0.0f, 0.0f, 0.0f)
	{
	}
};

		}

Aabb3 calculateModelBoundingBox(const Model& model)
{
	Aabb3 boundingBox;
	const AlignedVector< Vector4 >& positions = model.getPositions();
	for (AlignedVector< Vector4 >::const_iterator i = positions.begin(); i != positions.end(); ++i)
		boundingBox.contain(*i);
	return boundingBox;
}

namespace
{

	bool findBaseIndex(const Model& mdl, const Polygon& polygon, uint32_t& outBaseIndex)
	{
		outBaseIndex = c_InvalidIndex;

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
		{
			const Vertex* v[] =
			{
				&mdl.getVertex(vertices[i]),
				&mdl.getVertex(vertices[(i + 1) % vertices.size()]),
				&mdl.getVertex(vertices[(i + 2) % vertices.size()])
			};

			Vector4 p[] =
			{
				mdl.getPosition(v[0]->getPosition()),
				mdl.getPosition(v[1]->getPosition()),
				mdl.getPosition(v[2]->getPosition())
			};

			Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
			if (ep[0].length() > FUZZY_EPSILON && ep[1].length() > FUZZY_EPSILON && cross(ep[0], ep[1]).length() > FUZZY_EPSILON)
			{
				outBaseIndex = i;
				return true;
			}
		}

		return false;
	}

}

void calculateModelTangents(Model& mdl, bool binormals)
{
	const std::vector< Polygon >& polygons = mdl.getPolygons();
	AlignedVector< TangentBase > polygonTangentBases;
	AlignedVector< TangentBase > vertexTangentBases;
	uint32_t degenerated = 0;
	uint32_t invalid = 0;

	// Calculate tangent base for each polygon.
	polygonTangentBases.resize(polygons.size());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];
		uint32_t baseIndex;

		if (!findBaseIndex(mdl, polygon, baseIndex))
		{
			++degenerated;
			continue;
		}

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		T_ASSERT (vertices.size() == 3);

		const Vertex* v[] =
		{
			&mdl.getVertex(vertices[baseIndex]),
			&mdl.getVertex(vertices[(baseIndex + 1) % vertices.size()]),
			&mdl.getVertex(vertices[(baseIndex + 2) % vertices.size()])
		};

		Vector4 p[] =
		{
			mdl.getPosition(v[0]->getPosition()),
			mdl.getPosition(v[1]->getPosition()),
			mdl.getPosition(v[2]->getPosition())
		};

		Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
		T_ASSERT (ep[0].length() > FUZZY_EPSILON);
		T_ASSERT (ep[1].length() > FUZZY_EPSILON);

		ep[0] = ep[0].normalized();
		ep[1] = ep[1].normalized();

		TangentBase& tb = polygonTangentBases[i];
		tb.normal = cross(ep[0], ep[1]);
		tb.tangent = cross(Vector4(0.0f, 1.0f, 0.0f, 0.0f), tb.normal);
		tb.binormal = cross(tb.tangent, tb.normal);

		T_ASSERT (tb.normal.length() > FUZZY_EPSILON);
		tb.normal = tb.normal.normalized();
		
		if (v[0]->getTexCoord(0) != c_InvalidIndex && v[1]->getTexCoord(0) != c_InvalidIndex && v[2]->getTexCoord(0) != c_InvalidIndex)
		{
			Vector2 tc[] =
			{
				mdl.getTexCoord(v[0]->getTexCoord(0)),
				mdl.getTexCoord(v[1]->getTexCoord(0)),
				mdl.getTexCoord(v[2]->getTexCoord(0))
			};

			Vector2 etc[] = { tc[2] - tc[0], tc[1] - tc[0] };

			float denom = etc[0].x * etc[1].y - etc[1].x * etc[0].y;
			Scalar r = Scalar(denom != 0.0f ? 1.0f / denom : 0.0f);

			tb.tangent = ((Scalar(etc[0].y) * ep[1] - Scalar(etc[1].y) * ep[0]) * r).xyz0();
			tb.binormal = ((Scalar(etc[1].x) * ep[0] - Scalar(etc[0].x) * ep[1]) * r).xyz0();

			bool tangentValid = tb.tangent.length() > FUZZY_EPSILON;
			bool binormalValid = tb.binormal.length() > FUZZY_EPSILON;

			if (tangentValid || binormalValid)
			{
				if (!tangentValid)
					tb.tangent = cross(tb.binormal, tb.normal);
				if (!binormalValid)
					tb.binormal = cross(tb.tangent, tb.normal);
				
				tb.tangent = tb.tangent.normalized();
				tb.binormal = tb.binormal.normalized();

				Vector4 normal = cross(tb.tangent, tb.binormal);
				if (normal.length() >= FUZZY_EPSILON)
				{
					if (dot3(normal.normalized(), tb.normal) < 0.0f)
						tb.tangent = -tb.tangent;
				}
				else
					++invalid;
			}
			else
				++invalid;
		}
	}

	if (degenerated)
		log::warning << L"Degenerate " << degenerated << L" polygon(s) found in model" << Endl;

	if (invalid)
		log::warning << L"Invalid tangent space vectors; " << invalid << L" invalid texture coordinate(s)" << Endl;

	// Normalize polygon tangent bases.
	for (AlignedVector< TangentBase >::iterator i = polygonTangentBases.begin(); i != polygonTangentBases.end(); ++i)
	{
		if (i->normal.length() > FUZZY_EPSILON)
			i->normal = i->normal.normalized();
		if (i->tangent.length() > FUZZY_EPSILON)
			i->tangent = i->tangent.normalized();
		if (i->binormal.length() > FUZZY_EPSILON)
			i->binormal = i->binormal.normalized();
	}

	// Build new vertex normals.
	vertexTangentBases.resize(mdl.getVertexCount());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];

		const std::vector< uint32_t >& vertices = polygon.getVertices();
		for (std::vector< uint32_t >::const_iterator j = vertices.begin(); j != vertices.end(); ++j)
		{
			vertexTangentBases[*j].normal += polygonTangentBases[i].normal;
			vertexTangentBases[*j].tangent += polygonTangentBases[i].tangent;
			vertexTangentBases[*j].binormal += polygonTangentBases[i].binormal;
		}
	}

	// Normalize vertex tangent bases.
	for (AlignedVector< TangentBase >::iterator i = vertexTangentBases.begin(); i != vertexTangentBases.end(); ++i)
	{
		if (i->normal.length() > FUZZY_EPSILON)
			i->normal = i->normal.normalized();
		if (i->tangent.length() > FUZZY_EPSILON)
			i->tangent = i->tangent.normalized();
		if (i->binormal.length() > FUZZY_EPSILON)
			i->binormal = i->binormal.normalized();
	}

	// Update vertices.
	for (uint32_t i = 0; i < mdl.getVertexCount(); ++i)
	{
		const TangentBase& tb = vertexTangentBases[i];

		Vertex vertex = mdl.getVertex(i);
		if (vertex.getNormal() == c_InvalidIndex)
			vertex.setNormal(mdl.addUniqueNormal(tb.normal));
		if (vertex.getTangent() == c_InvalidIndex)
			vertex.setTangent(mdl.addUniqueNormal(tb.tangent));
		if (vertex.getBinormal() == c_InvalidIndex)
			vertex.setBinormal(mdl.addUniqueNormal(tb.binormal));

		mdl.setVertex(i, vertex);
	}
}

void triangulateModel(Model& model)
{
	std::vector< Polygon > triangulatedPolygons;
	std::vector< Triangulator::Triangle > triangles;

	const std::vector< Polygon >& polygons = model.getPolygons();
	triangulatedPolygons.reserve(polygons.size());

	for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		const std::vector< uint32_t >& vertices = i->getVertices();
		if (vertices.size() > 3)
		{
			Winding3 polygonWinding;

			polygonWinding.points.resize(vertices.size());
			for (size_t j = 0; j < vertices.size(); ++j)
				polygonWinding.points[j] = model.getPosition(model.getVertex(vertices[j]).getPosition());

			Vector4 polygonNormal;
			if (i->getNormal() != c_InvalidIndex)
				polygonNormal = model.getNormal(i->getNormal());
			else
			{
				// No normal associated with polygon; try to determine from winding.
				Plane polygonPlane;
				if (!polygonWinding.getPlane(polygonPlane))
					continue;

				polygonNormal = polygonPlane.normal();
			}

			triangles.resize(0);
			Triangulator().freeze(
				polygonWinding.points,
				polygonNormal,
				triangles
			);

			for (std::vector< Triangulator::Triangle >::const_iterator j = triangles.begin(); j != triangles.end(); ++j)
			{
				Polygon triangulatedPolygon(
					i->getMaterial(),
					vertices[j->indices[0]],
					vertices[j->indices[1]],
					vertices[j->indices[2]]
				);
				triangulatedPolygons.push_back(triangulatedPolygon);
			}
		}
		else if (vertices.size() == 3)
			triangulatedPolygons.push_back(*i);
	}

	model.setPolygons(triangulatedPolygons);
}

namespace
{

	struct HullFace
	{
		uint32_t i[3];

		HullFace()
		{
			i[0] = ~0U; i[1] = ~0U; i[2] = ~0U;
		}

		HullFace(
			uint32_t i1, uint32_t i2, uint32_t i3
		)
		{
			i[0] = i1; i[1] = i2; i[2] = i3;
		}
	};

	struct HullFaceAdjacency
	{
		uint32_t n[3];

		HullFaceAdjacency()
		{
			n[0] = ~0U; n[1] = ~0U; n[2] = ~0U;
		}

		HullFaceAdjacency(
			uint32_t n1, uint32_t n2, uint32_t n3
		)
		{
			n[0] = n1; n[1] = n2; n[2] = n3;
		}
	};

	uint32_t calculateAdjacency(const std::vector< HullFace >& faces, std::vector< HullFaceAdjacency >& outAdjacency)
	{
		uint32_t errorCount = 0;
		for (uint32_t i = 0; i < uint32_t(faces.size()); ++i)
		{
			HullFaceAdjacency adj;

			for (uint32_t j = 0; j < 3; ++j)
			{
				int a1 = faces[i].i[j];
				int a2 = faces[i].i[(j + 1) % 3];

				for (uint32_t k = 0; k < uint32_t(faces.size()) && adj.n[j] == ~0U; ++k)
				{
					if (i == k)
						continue;

					for (uint32_t m = 0; m < 3; ++m)
					{
						int na1 = faces[k].i[m];
						int na2 = faces[k].i[(m + 1) % 3];

						T_ASSERT_M(!(a1 == na1 && a2 == na2), L"Adjacent face with different winding");

						if (a1 == na2 && a2 == na1)
						{
							adj.n[j] = k;
							break;
						}
					}
				}
			}

			if (adj.n[0] == ~0U || adj.n[1] == ~0U || adj.n[2] == ~0U)
				++errorCount;

			outAdjacency.push_back(adj);
		}
		return errorCount;
	}

}

void calculateConvexHull(Model& model)
{
	uint32_t errorCount = 0;

	AlignedVector< Vector4 > vertices = model.getPositions();

	// Use first polygon as basis for tetrahedron.
	Polygon p0 = model.getPolygons().front();

	uint32_t i0 = model.getVertex(p0.getVertex(0)).getPosition();
	uint32_t i1 = model.getVertex(p0.getVertex(1)).getPosition();
	uint32_t i2 = model.getVertex(p0.getVertex(2)).getPosition();

	Plane pll(
		vertices[i0],
		vertices[i1],
		vertices[i2]
	);

	// Build initial tetrahedron.
	uint32_t it = ~0U;
	for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
	{
		if (i == i0 || i == i1 || i == i2)
			continue;

		Scalar d = pll.distance(vertices[i]);
		if (d < -FUZZY_EPSILON)
		{
			it = i;
			break;
		}
	}
	if (it == ~0U)
		return;

	std::vector< HullFace > faces;
	faces.reserve(32);
	faces.push_back(HullFace(i0, i1, i2));
	faces.push_back(HullFace(i1, i0, it));
	faces.push_back(HullFace(i2, i1, it));
	faces.push_back(HullFace(i0, i2, it));

	std::vector< HullFaceAdjacency > adjacency;
	errorCount = calculateAdjacency(faces, adjacency);

	if (errorCount > 0)
		T_DEBUG(errorCount << L" adjanceny error(s)");

	for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
	{
		std::vector< bool > visible(faces.size());
		for (uint32_t j = 0; j < uint32_t(faces.size()); ++j)
		{
			Plane pl(
				vertices[faces[j].i[0]],
				vertices[faces[j].i[1]],
				vertices[faces[j].i[2]]
			);
			visible[j] = bool(pl.distance(vertices[i]) >= -FUZZY_EPSILON);
		}

		std::vector< std::pair< uint32_t, uint32_t > > silouette;
		for (uint32_t j = 0; j < uint32_t(faces.size()); ++j)
		{
			for (uint32_t k = 0; k < 3; ++k)
			{
				uint32_t n = adjacency[j].n[k];
				if (n != ~0UL && !visible[j] && visible[n])
				{
					silouette.push_back(std::make_pair(
						faces[j].i[k],
						faces[j].i[(k + 1) % 3]
					));
				}
			}
		}
		if (silouette.empty())
			continue;

		//T_ASSERT (silouette.size() >= 3);

		//std::vector< std::pair< uint32_t, uint32_t > > silouette2;
		//silouette2.push_back(silouette.front());

		//while (silouette2.size() != silouette.size())
		//{
		//	for (std::vector< std::pair< uint32_t, uint32_t > >::const_iterator j = silouette.begin(); j != silouette.end(); ++j)
		//	{
		//		if (silouette2.back().second == j->first)
		//		{
		//			silouette2.push_back(*j);
		//			break;
		//		}
		//	}
		//}

		//silouette = silouette2;

		// Remove visible faces.
		for (uint32_t j = 0; j < uint32_t(visible.size()); )
		{
			if (visible[j])
			{
				faces.erase(faces.begin() + j);
				visible.erase(visible.begin() + j);
			}
			else
				++j;
		}

		// Add new faces.
		for (std::vector< std::pair< uint32_t, uint32_t > >::iterator j = silouette.begin(); j != silouette.end(); ++j)
		{
			int idx[] = { j->second, j->first, i };
			faces.push_back(HullFace(idx[0], idx[1], idx[2]));
		}
		
		// Recalculate adjacency.
		adjacency.resize(0);
		errorCount = calculateAdjacency(faces, adjacency);
		if (errorCount > 0)
			T_DEBUG(errorCount << L" adjanceny error(s)");
	}

	// Clear everything except positions.
	model.clear(Model::CfMaterials | Model::CfVertices | Model::CfPolygons | Model::CfNormals | Model::CfTexCoords | Model::CfBones);

	for (std::vector< HullFace >::iterator i = faces.begin(); i != faces.end(); ++i)
	{
		Vector4 v[] =
		{
			vertices[i->i[0]],
			vertices[i->i[1]],
			vertices[i->i[2]]
		};

		Vector4 e[] =
		{
			v[1] - v[0],
			v[2] - v[0]
		};

		if (cross(e[0], e[1]).length() <= FUZZY_EPSILON)
			continue;

		Polygon polygon;
		for (uint32_t j = 0; j < 3; ++j)
		{
			Vertex vertex;
			vertex.setPosition(i->i[j]);
			polygon.addVertex(model.addVertex(vertex));
		}

		model.addPolygon(polygon);
	}

	if (errorCount > 0)
		log::warning << L"Unable to build complete adjacency of " << errorCount << L" face(s)" << Endl;
}

namespace
{

	struct SortPolygonDistance
	{
		bool operator () (const Polygon& a, const Polygon& b) const
		{
			const std::vector< uint32_t >& va = a.getVertices();
			const std::vector< uint32_t >& vb = b.getVertices();

			uint32_t ia = std::accumulate(va.begin(), va.end(), 0);
			uint32_t ib = std::accumulate(vb.begin(), vb.end(), 0);

			return ia < ib;
		}
	};

}

void sortPolygonsCacheCoherent(Model& model)
{
	std::vector< Polygon > polygons = model.getPolygons();
	if (polygons.size() <= 2)
		return;

	std::vector< Polygon > newPolygons;
	newPolygons.reserve(polygons.size());

	uint32_t materialCount = model.getMaterials().size();
	for (uint32_t material = 0; material < materialCount; ++material)
	{
		std::vector< uint32_t > indices;
		for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
		{
			if (i->getMaterial() == material)
				indices.insert(indices.end(), i->getVertices().begin(), i->getVertices().end());
		}

		if (indices.empty())
			continue;

		std::vector< uint32_t >::const_iterator it = std::max_element(indices.begin(), indices.end());
		uint32_t vertexCount = *it + 1;

		std::vector< uint32_t > newIndices(indices.size());
		optimizeFaces(
			indices,
			vertexCount,
			newIndices,
			32
		);

		for (uint32_t i = 0; i < newIndices.size(); i += 3)
		{
			newPolygons.push_back(Polygon(
				material,
				newIndices[i + 0],
				newIndices[i + 1],
				newIndices[i + 2]
			));
		}
	}

	model.setPolygons(newPolygons);
}

void cleanDuplicates(Model& model)
{
	std::vector< Vertex > vertices = model.getVertices();
	std::vector< Polygon > polygons = model.getPolygons();

	for (std::vector< Vertex >::iterator i = vertices.begin(); i != vertices.end() - 1; )
	{
		uint32_t duplicateIndex = c_InvalidIndex;
		for (std::vector< Vertex >::iterator j = i + 1; j != vertices.end(); ++j)
		{
			if (*i == *j)
			{
				duplicateIndex = uint32_t(std::distance(vertices.begin(), j));
				break;
			}
		}
		if (duplicateIndex != c_InvalidIndex)
		{
			uint32_t originalIndex = uint32_t(std::distance(vertices.begin(), i));

			for (std::vector< Polygon >::iterator j = polygons.begin(); j != polygons.end(); ++j)
			{
				for (uint32_t k = 0; k < j->getVertexCount(); ++k)
				{
					if (j->getVertex(k) == originalIndex)
						j->setVertex(k, duplicateIndex);
				}
			}

			i = vertices.erase(i);
		}
		else
			i++;
	}

	for (std::vector< Polygon >::iterator i = polygons.begin(); i != polygons.end() - 1; )
	{
		bool duplicate = false;
		for (std::vector< Polygon >::iterator j = i + 1; j != polygons.end(); ++j)
		{
			if (*i == *j)
			{
				duplicate = true;
				break;
			}
		}
		if (duplicate)
			i = polygons.erase(i);
		else
			i++;
	}

	model.setVertices(vertices);
	model.setPolygons(polygons);
}

void flattenDoubleSided(Model& model)
{
	std::vector< Polygon > polygons = model.getPolygons();
	std::vector< Polygon > flatten; flatten.reserve(polygons.size());

	for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		uint32_t materialId = i->getMaterial();
		const Material& material = model.getMaterial(materialId);
		if (!material.isDoubleSided())
			continue;

		Polygon flat = *i;
		for (uint32_t j = 0; j < i->getVertexCount(); ++j)
		{
			uint32_t vtx = i->getVertex(j);
			flat.setVertex(i->getVertexCount() - j - 1, vtx);
		}
		flatten.push_back(flat);
	}

	flatten.insert(flatten.end(), polygons.begin(), polygons.end());
	model.setPolygons(flatten);

	std::vector< Material > materials = model.getMaterials();
	for (std::vector< Material >::iterator i = materials.begin(); i != materials.end(); ++i)
		i->setDoubleSided(false);
	model.setMaterials(materials);
}

void bakeVertexOcclusion(Model& model)
{
	const uint32_t c_occlusionRayCount = 64;
	const Scalar c_occlusionRaySpread(0.75f);
	const Scalar c_occlusionRayBias(0.1f);

	RandomGeometry rnd;

	const std::vector< Polygon >& polygons = model.getPolygons();
	std::vector< Vertex > vertices = model.getVertices();
	
	AlignedVector< Vector4 > colors = model.getColors();
	model.clear(Model::CfColors);
	
	AlignedVector< Winding3 > windings(polygons.size());
	for (uint32_t i = 0; i < polygons.size(); ++i)
	{
		Winding3& w = windings[i];
		const std::vector< uint32_t >& vertexIndices = polygons[i].getVertices();
		for (std::vector< uint32_t >::const_iterator j = vertexIndices.begin(); j != vertexIndices.end(); ++j)
		{
			const Vertex& polyVertex = model.getVertex(*j);
			const Vector4& polyVertexPosition = model.getPosition(polyVertex.getPosition());
			w.points.push_back(polyVertexPosition);
		}
	}

	// Build acceleration tree.
	SahTree sah;
	sah.build(windings);
	
	for (std::vector< Vertex >::iterator i = vertices.begin(); i != vertices.end(); ++i)
	{
		const Vector4& position = model.getPosition(i->getPosition());
		const Vector4& normal = model.getNormal(i->getNormal());
		
		uint32_t occluded = 0;
		for (uint32_t j = 0; j < c_occlusionRayCount; ++j)
		{
			Vector4 rayDirection = lerp(normal, rnd.nextHemi(normal), c_occlusionRaySpread).normalized().xyz0();
			Vector4 rayOrigin = (position + normal * c_occlusionRayBias).xyz1();
			if (sah.queryAnyIntersection(rayOrigin, rayDirection, 0.0f))
				occluded++;
		}
		
		Vector4 color = Vector4::one();
		
		if (i->getColor() != c_InvalidIndex)
			color = colors[i->getColor()];
			
		color.set(3, Scalar(
			1.0f - occluded / float(c_occlusionRayCount)
		));
		
		i->setColor(model.addUniqueColor(color));
	}
	
	model.setVertices(vertices);
}

namespace
{

struct BakePixelVisitor
{
	RandomGeometry& random;
	const SahTree& sah;
	Vector4 P[3];
	Vector4 N[3];
	drawing::Image& outImage;

	BakePixelVisitor(
		RandomGeometry& _random,
		const SahTree& _sah,
		const Vector4 _P[3],
		const Vector4 _N[3],
		drawing::Image& _outImage
	)
	:	random(_random)
	,	sah(_sah)
	,	outImage(_outImage)
	{
		for (int i = 0; i < 3; ++i)
		{
			P[i] = _P[i];
			N[i] = _N[i];
		}
	}

	void operator () (int32_t x, int32_t y, float alpha, float beta, float gamma)
	{
		const uint32_t c_occlusionRayCount = 64;
		const Scalar c_occlusionRaySpread(0.75f);
		const static Scalar c_occlusionRayBias(0.1f);

		Vector4 position = (P[0] * Scalar(alpha) + P[1] * Scalar(beta) + P[2] * Scalar(gamma)).xyz1();
		Vector4 normal = (N[0] * Scalar(alpha) + N[1] * Scalar(beta) + N[2] * Scalar(gamma)).xyz0();

		uint32_t occluded = 0;
		for (uint32_t j = 0; j < c_occlusionRayCount; ++j)
		{
			Vector4 rayDirection = lerp(normal, random.nextHemi(normal), c_occlusionRaySpread).normalized().xyz0();
			Vector4 rayOrigin = (position + normal * c_occlusionRayBias).xyz1();
			if (sah.queryAnyIntersection(rayOrigin, rayDirection, 0.0f))
				occluded++;
		}

		float occf = 1.0f - float(occluded) / c_occlusionRayCount;
		outImage.setPixel(x, y, Color4f(occf, occf, occf, occf));
	}
};

}

Ref< drawing::Image > bakePixelOcclusion(Model& model, int32_t width, int32_t height)
{
	const uint32_t c_occlusionRayCount = 64;
	const Scalar c_occlusionRaySpread(0.75f);
	const Scalar c_occlusionRayBias(0.1f);

	RandomGeometry rnd;

	const std::vector< Polygon >& polygons = model.getPolygons();
	std::vector< Vertex > vertices = model.getVertices();

	AlignedVector< Vector4 > colors = model.getColors();
	model.clear(Model::CfColors);

	AlignedVector< Winding3 > windings(polygons.size());
	for (uint32_t i = 0; i < polygons.size(); ++i)
	{
		Winding3& w = windings[i];
		const std::vector< uint32_t >& vertexIndices = polygons[i].getVertices();
		for (std::vector< uint32_t >::const_iterator j = vertexIndices.begin(); j != vertexIndices.end(); ++j)
		{
			const Vertex& polyVertex = model.getVertex(*j);
			const Vector4& polyVertexPosition = model.getPosition(polyVertex.getPosition());
			w.points.push_back(polyVertexPosition);
		}
	}

	// Build acceleration tree.
	SahTree sah;
	sah.build(windings);

	// Create occlusion image.
	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getX8R8G8B8(), width, height);
	image->clear(Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	// Trace each polygon in UV space.
	Vector2 dim(width, height);
	Vector2 uv[3];
	Vector4 P[3], N[3];

	for (uint32_t i = 0; i < polygons.size(); ++i)
	{
		const Vertex& v0 = vertices[polygons[i].getVertex(0)];
		const Vertex& v1 = vertices[polygons[i].getVertex(1)];
		const Vertex& v2 = vertices[polygons[i].getVertex(2)];

		int32_t tc0 = v0.getTexCoord(0);
		int32_t tc1 = v1.getTexCoord(0);
		int32_t tc2 = v2.getTexCoord(0);

		if (tc0 == c_InvalidIndex || tc1 == c_InvalidIndex || tc2 == c_InvalidIndex)
			continue;

		uv[0] = model.getTexCoord(tc0) * dim;
		uv[1] = model.getTexCoord(tc1) * dim;
		uv[2] = model.getTexCoord(tc2) * dim;

		P[0] = model.getPosition(v0.getPosition());
		P[1] = model.getPosition(v1.getPosition());
		P[2] = model.getPosition(v2.getPosition());

		int32_t n0 = v0.getNormal();
		int32_t n1 = v1.getNormal();
		int32_t n2 = v2.getNormal();

		if (n0 == c_InvalidIndex || n1 == c_InvalidIndex || n2 == c_InvalidIndex)
			continue;

		N[0] = model.getNormal(n0);
		N[1] = model.getNormal(n1);
		N[2] = model.getNormal(n2);

		BakePixelVisitor visitor(rnd, sah, P, N, *image);
		triangle(uv[0], uv[1], uv[2], visitor);
	}

	return image;
}

void cullDistantFaces(Model& model)
{
	Aabb3 viewerRegion(
		Vector4(-40.0f, -40.0f, -40.0f),
		Vector4( 40.0f,  40.0f,  40.0f)
	);

	Vector4 viewerCorners[8];
	viewerRegion.getExtents(viewerCorners);

	std::vector< Polygon > polygons = model.getPolygons();
	uint32_t originalCount = polygons.size();

	for (uint32_t i = 0; i < polygons.size(); )
	{
		uint32_t vertexCount = polygons[i].getVertexCount();
		if (vertexCount < 3)
		{
			++i;
			continue;
		}

		Winding3 winding;
		for (uint32_t j = 0; j < vertexCount; ++j)
		{
			const Vertex& vertex = model.getVertex(polygons[i].getVertex(j));
			const Vector4& position = model.getPosition(vertex.getPosition());
			winding.points.push_back(position);
		}

		Plane plane;
		if (!winding.getPlane(plane))
		{
			++i;
			continue;
		}

		Vector4 normal = plane.normal();

		bool visible = false;
		for (uint32_t j = 0; j < 8 && !visible; ++j)
		{
			for (uint32_t k = 0; k < vertexCount && !visible; ++k)
			{
				const Vertex& vertex = model.getVertex(polygons[i].getVertex(k));
				const Vector4& position = model.getPosition(vertex.getPosition());
				if (dot3(viewerCorners[j] - position, normal) > 0.0f)
					visible = true;
			}
		}

		if (!visible)
			polygons.erase(polygons.begin() + i);
		else
			++i;
	}
	model.setPolygons(polygons);

	log::info << L"Culled " << uint32_t(originalCount - polygons.size()) << L" polygon(s)" << Endl;
}

void mergeModels(Model& model, const Model& sourceModel, const Transform& sourceModelTransform)
{
	int32_t materialBase = model.getMaterials().size();

	// Merge materials.
	const std::vector< Material >& sourceMaterials = sourceModel.getMaterials();
	for (std::vector< Material >::const_iterator i = sourceMaterials.begin(); i != sourceMaterials.end(); ++i)
		model.addMaterial(*i);

	// Merge geometry.
	const std::vector< Vertex >& sourceVertices = sourceModel.getVertices();

	std::vector< uint32_t > vertexMap;
	vertexMap.resize(sourceVertices.size(), c_InvalidIndex);

	for (size_t i = 0; i < sourceVertices.size(); ++i)
	{
		const Vertex& sourceVertex = sourceVertices[i];

		uint32_t position = sourceVertex.getPosition();
		if (position != c_InvalidIndex)
			position = model.addUniquePosition(sourceModelTransform * sourceModel.getPosition(position).xyz1());
		
		uint32_t color = sourceVertex.getColor();
		if (color != c_InvalidIndex)
			color = model.addUniqueColor(sourceModel.getColor(color));

		uint32_t normal = sourceVertex.getNormal();
		if (normal != c_InvalidIndex)
			normal = model.addUniqueNormal(sourceModelTransform * sourceModel.getNormal(normal).xyz0());

		uint32_t tangent = sourceVertex.getTangent();
		if (tangent != c_InvalidIndex)
			tangent = model.addUniqueNormal(sourceModelTransform * sourceModel.getNormal(tangent).xyz0());

		uint32_t binormal = sourceVertex.getBinormal();
		if (binormal != c_InvalidIndex)
			binormal = model.addUniqueNormal(sourceModelTransform * sourceModel.getNormal(binormal).xyz0());

		Vertex v;
		v.setPosition(position);
		v.setColor(color);
		v.setNormal(normal);
		v.setTangent(tangent);
		v.setBinormal(binormal);

		vertexMap[i] = model.addUniqueVertex(v);
	}

	// Merge polygons.
	const std::vector< Polygon >& sourcePolygons = sourceModel.getPolygons();
	for (std::vector< Polygon >::const_iterator i = sourcePolygons.begin(); i != sourcePolygons.end(); ++i)
	{
		Polygon p;
		p.setMaterial(materialBase + i->getMaterial());

		const std::vector< uint32_t >& sourceVertices = i->getVertices();
		for (uint32_t j = 0; j < sourceVertices.size(); ++j)
			p.addVertex(vertexMap[sourceVertices[j]]);

		if (i->getNormal() != c_InvalidIndex)
		{
			uint32_t normal = model.addUniqueNormal(sourceModelTransform * sourceModel.getNormal(i->getNormal()).xyz0());
			p.setNormal(normal);
		}

		model.addUniquePolygon(p);
	}
}

	}
}
