#pragma once

#include <map>
#include "Core/Guid.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

/*! Mesh asset.
 * \ingroup Mesh
 */
class T_DLLCLASS MeshAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	enum MeshType
	{
		MtBlend = 0,
		MtIndoor = 1,
		MtInstance = 2,
		MtLod = 3,
		MtPartition = 4,
		MtSkinned = 5,
		MtStatic = 6,
		MtStream = 7
	};

	MeshAsset();

	virtual void serialize(ISerializer& s) override final;

	/*! Set type of runtime mesh. */
	void setMeshType(MeshType meshType) { m_meshType = meshType; }

	/*! Get type of runtime mesh. */
	MeshType getMeshType() const { return m_meshType; }

	/*! Set material shader templates. */
	void setMaterialTemplates(const std::map< std::wstring, Guid >& materialTemplates) { m_materialTemplates = materialTemplates; }

	/*! Get material shader templates. */
	const std::map< std::wstring, Guid >& getMaterialTemplates() const { return m_materialTemplates; }

	/*! Set explicit material shaders. */
	void setMaterialShaders(const std::map< std::wstring, Guid >& materialShaders) { m_materialShaders = materialShaders; }

	/*! Get explicit material shaders. */
	const std::map< std::wstring, Guid >& getMaterialShaders() const { return m_materialShaders; }

	/*! Set material textures. */
	void setMaterialTextures(const std::map< std::wstring, Guid >& materialTextures) { m_materialTextures = materialTextures; }

	/*! Get material textures. */
	const std::map< std::wstring, Guid >& getMaterialTextures() const { return m_materialTextures; }

	/*! Set scale factor. */
	void setScaleFactor(float scaleFactor) { m_scaleFactor = scaleFactor; }

	/*! Get scale factor. */
	float getScaleFactor() const { return m_scaleFactor; }

	/*! Set if model should be centered around origo before converted. */
	void setCenter(bool center) { m_center = center; }

	/*! Check if model should be centered around origo. */
	bool getCenter() const { return m_center; }

	/*! Set number of manual lod steps. */
	void setLodSteps(int32_t lodSteps) { m_lodSteps = lodSteps; }

	/*! Get number of manual lod steps. */
	int32_t getLodSteps() const { return m_lodSteps; }

	/*! Set distance when lowest lod should be used. */
	void setLodMaxDistance(float lodMaxDistance) { m_lodMaxDistance = lodMaxDistance; }

	/*! Get distance when lowest lod should be used. */
	float getLodMaxDistance() const { return m_lodMaxDistance; }

	/*! Set distance to when mesh should no longer be rendererd. */
	void setLodCullDistance(float lodCullDistance) { m_lodCullDistance = lodCullDistance; }

	/*! Get distance to when mesh should no longer be rendererd. */
	float getLodCullDistance() const { return m_lodCullDistance; }

private:
	MeshType m_meshType;
	std::map< std::wstring, Guid > m_materialTemplates;
	std::map< std::wstring, Guid > m_materialShaders;
	std::map< std::wstring, Guid > m_materialTextures;
	float m_scaleFactor;
	bool m_center;
	int32_t m_lodSteps;
	float m_lodMaxDistance;
	float m_lodCullDistance;
};

	}
}

