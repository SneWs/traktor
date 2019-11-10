#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Model/Model.h"
#include "Model/Operations/MergeModel.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

const Scalar c_snapDistance(0.02f);
const Scalar c_snapDistanceSqr(c_snapDistance * c_snapDistance);

int32_t predicateAxis(float a, float b)
{
	float dab = a - b;
	if (dab < -FUZZY_EPSILON)
		return -1;
	else if (dab > FUZZY_EPSILON)
		return 1;
	else
		return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.MergeModel", MergeModel, IModelOperation)

MergeModel::MergeModel(const Model& sourceModel, const Transform& sourceTransform, float positionDistance)
:	m_sourceModel(sourceModel)
,	m_sourceTransform(sourceTransform)
,	m_positionDistance(positionDistance)
{
}

bool MergeModel::apply(Model& model) const
{
	SmallMap< uint32_t, uint32_t > materialMap;
	SmallMap< uint32_t, uint32_t > channelMap;

	// Merge texture channels.
	for (uint32_t i = 0; i < m_sourceModel.getTexCoordChannels().size(); ++i)
		channelMap[i] = model.addUniqueTexCoordChannel(m_sourceModel.getTexCoordChannels()[i]);

	// Merge materials.
	for (uint32_t i = 0; i < m_sourceModel.getMaterialCount(); ++i)
	{
		Material material = m_sourceModel.getMaterial(i);

		// Ensure texture channels are remapped in material before adding.
		Material::Map mm[] =
		{
			material.getDiffuseMap(),
			material.getSpecularMap(),
			material.getRoughnessMap(),
			material.getMetalnessMap(),
			material.getTransparencyMap(),
			material.getEmissiveMap(),
			material.getReflectiveMap(),
			material.getNormalMap(),
			material.getLightMap()
		};
		for (auto& m : mm)
		{
			if (!m.name.empty())
				m.channel = channelMap[m.channel];
		}
		material.setDiffuseMap(mm[0]);
		material.setSpecularMap(mm[1]);
		material.setRoughnessMap(mm[2]);
		material.setMetalnessMap(mm[3]);
		material.setTransparencyMap(mm[4]);
		material.setEmissiveMap(mm[5]);
		material.setReflectiveMap(mm[6]);
		material.setNormalMap(mm[7]);
		material.setLightMap(mm[8]);

		materialMap[i] = model.addUniqueMaterial(material);
	}

	// Merge geometry.
	const AlignedVector< Polygon >& sourcePolygons = m_sourceModel.getPolygons();
	const AlignedVector< Vertex >& sourceVertices = m_sourceModel.getVertices();

	AlignedVector< uint32_t > vertexMap;
	vertexMap.resize(sourceVertices.size(), c_InvalidIndex);

	model.reservePositions(model.getPositionCount() + m_sourceModel.getPositionCount());
	model.reserveColors(model.getColorCount() + m_sourceModel.getColorCount());
	model.reserveNormals(model.getNormalCount() + m_sourceModel.getNormalCount());
	model.reserveVertices(model.getVertexCount() + m_sourceModel.getVertexCount());

	for (size_t i = 0; i < sourceVertices.size(); ++i)
	{
		const Vertex& sourceVertex = sourceVertices[i];

		uint32_t position = sourceVertex.getPosition();
		if (position != c_InvalidIndex)
			position = model.addUniquePosition(m_sourceTransform * m_sourceModel.getPosition(position).xyz1(), m_positionDistance);

		uint32_t color = sourceVertex.getColor();
		if (color != c_InvalidIndex)
			color = model.addUniqueColor(m_sourceModel.getColor(color));

		uint32_t normal = sourceVertex.getNormal();
		if (normal != c_InvalidIndex)
			normal = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(normal).xyz0());

		uint32_t tangent = sourceVertex.getTangent();
		if (tangent != c_InvalidIndex)
			tangent = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(tangent).xyz0());

		uint32_t binormal = sourceVertex.getBinormal();
		if (binormal != c_InvalidIndex)
			binormal = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(binormal).xyz0());

		Vertex v;
		v.setPosition(position);
		v.setColor(color);
		v.setNormal(normal);
		v.setTangent(tangent);
		v.setBinormal(binormal);

		for (uint32_t j = 0; j < sourceVertex.getTexCoordCount(); ++j)
		{
			uint32_t texCoord = sourceVertex.getTexCoord(j);
			if (texCoord != c_InvalidIndex)
			{
				texCoord = model.addUniqueTexCoord(m_sourceModel.getTexCoord(texCoord));
				v.setTexCoord(channelMap[j], texCoord);
			}
		}

		for (uint32_t j = 0; j < sourceVertex.getJointInfluenceCount(); ++j)
		{
			float influence = sourceVertex.getJointInfluence(j);
			v.setJointInfluence(j, influence);
		}

		vertexMap[i] = model.addUniqueVertex(v);
	}

	AlignedVector< Polygon >& mergedPolygons = model.getPolygons();
	mergedPolygons.reserve(mergedPolygons.size() + sourcePolygons.size());

	AlignedVector< Polygon > outputPolygons;
	outputPolygons.reserve(sourcePolygons.size());

	for (size_t i = 0; i < sourcePolygons.size(); ++i)
	{
		const Polygon& sourcePolygon = sourcePolygons[i];
		const AlignedVector< uint32_t >& sourceVertices = sourcePolygon.getVertices();

		// Remap vertices.
		AlignedVector< uint32_t > outputVertices(sourceVertices.size());
		for (size_t j = 0; j < sourceVertices.size(); ++j)
			outputVertices[j] = vertexMap[sourceVertices[j]];

		// Rotate vertices; keep vertex with lowest geometrical position first.
		uint32_t minPositionIndex = ~0U;
		Vector4 minPosition = Vector4(
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max()
		);

		for (size_t j = 0; j < outputVertices.size(); ++j)
		{
			const Vector4& position = model.getVertexPosition(outputVertices[j]);

			int32_t cx = predicateAxis(position.x(), minPosition.x());
			int32_t cy = predicateAxis(position.y(), minPosition.y());
			int32_t cz = predicateAxis(position.z(), minPosition.z());

			if (
				cx < 0 ||
				(cx == 0 && cy < 0) ||
				(cx == 0 && cy == 0 && cz < 0)
			)
			{
				minPositionIndex = j;
				minPosition = position;
			}
		}

		std::rotate(outputVertices.begin(), outputVertices.begin() + (size_t)minPositionIndex, outputVertices.end());

		// Create polygon.
		Polygon outputPolygon;
		outputPolygon.setMaterial(materialMap[sourcePolygon.getMaterial()]);

		if (sourcePolygon.getNormal() != c_InvalidIndex)
		{
			uint32_t normal = model.addUniqueNormal(m_sourceTransform * m_sourceModel.getNormal(sourcePolygon.getNormal()).xyz0());
			outputPolygon.setNormal(normal);
		}

		outputPolygon.setVertices(outputVertices);

		// Check if polygon is duplicated or canceling through opposite winding.
		bool duplicate = false;

		for (size_t j = 0; j < mergedPolygons.size(); ++j)
		{
			const Polygon& mergedPolygon = mergedPolygons[j];

			if (mergedPolygon.getVertexCount() != outputPolygon.getVertexCount())
				continue;

			uint32_t outputVertex = outputPolygon.getVertex(0);
			uint32_t mergedVertex = mergedPolygon.getVertex(0);

			const Vector4& outputPosition = model.getVertexPosition(outputVertex);
			const Vector4& mergedPosition = model.getVertexPosition(mergedVertex);

			Scalar distance = (outputPosition - mergedPosition).xyz0().length2();
			if (distance > c_snapDistanceSqr)
				continue;

			uint32_t vertexCount = outputPolygon.getVertexCount();

			duplicate = true;

			for (uint32_t k = 1; k < vertexCount; ++k)
			{
				uint32_t outputVertex = outputPolygon.getVertex(k);
				uint32_t mergedVertex = mergedPolygon.getVertex(k);

				const Vector4& outputPosition = model.getVertexPosition(outputVertex);
				const Vector4& mergedPosition = model.getVertexPosition(mergedVertex);

				Scalar distance = (outputPosition - mergedPosition).xyz0().length2();
				if (distance > c_snapDistanceSqr)
				{
					duplicate = false;
					break;
				}
			}

			if (duplicate)
				break;

			duplicate = true;

			for (uint32_t k = 1; k < vertexCount; ++k)
			{
				uint32_t outputVertex = outputPolygon.getVertex(k);
				uint32_t mergedVertex = mergedPolygon.getVertex(vertexCount - k);

				const Vector4& outputPosition = model.getVertexPosition(outputVertex);
				const Vector4& mergedPosition = model.getVertexPosition(mergedVertex);

				Scalar distance = (outputPosition - mergedPosition).xyz0().length2();
				if (distance > c_snapDistanceSqr)
				{
					duplicate = false;
					break;
				}
			}

			if (duplicate)
			{
				mergedPolygons.erase(mergedPolygons.begin() + j);
				break;
			}
		}

		if (!duplicate)
			outputPolygons.push_back(outputPolygon);
	}

	mergedPolygons.insert(mergedPolygons.end(), outputPolygons.begin(), outputPolygons.end());
	return true;
}

	}
}
