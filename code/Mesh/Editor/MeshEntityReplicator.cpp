#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshEntityReplicator.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/Transform.h"
#include "Render/Editor/Texture/TextureSet.h"

namespace traktor
{
    namespace mesh
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityReplicator", 0, MeshEntityReplicator, scene::IEntityReplicator)

bool MeshEntityReplicator::create(const editor::IPipelineSettings* settings)
{
	m_modelCachePath = settings->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

TypeInfoSet MeshEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< MeshComponentData >();
}

Ref< model::Model > MeshEntityReplicator::createModel(
    editor::IPipelineBuilder* pipelineBuilder,
	const std::wstring& assetPath,
    const Object* source
) const
{
	const MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const MeshComponentData* >(source);

	// Get referenced mesh asset.
	Ref< MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< MeshAsset >(meshComponentData->getMesh());
	if (!meshAsset)
		return nullptr;

	// Read source model.
	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + meshAsset->getFileName());
	Ref< model::Model > model = model::ModelCache(m_modelCachePath).get(filePath, meshAsset->getImportFilter());
	if (!model)
		return nullptr;

	model::Transform(scale(meshAsset->getScaleFactor(), meshAsset->getScaleFactor(), meshAsset->getScaleFactor())).apply(*model);

	// Create list of texture references.
	std::map< std::wstring, Guid > materialTextures;

	// First use textures from texture set.
	const auto& textureSetId = meshAsset->getTextureSet();
	if (textureSetId.isNotNull())
	{
		Ref< const render::TextureSet > textureSet = pipelineBuilder->getObjectReadOnly< render::TextureSet >(textureSetId);
		if (!textureSet)
			return nullptr;

		materialTextures = textureSet->get();
	}

	// Then let explicit material textures override those from a texture set.
	for (const auto& mt : meshAsset->getMaterialTextures())
		materialTextures[mt.first] = mt.second;

	// Bind texture references in material maps.
	for (auto& material : model->getMaterials())
	{
		auto diffuseMap = material.getDiffuseMap();
		auto it = materialTextures.find(diffuseMap.name);
		if (it != materialTextures.end())
		{
			diffuseMap.texture = it->second;
			material.setDiffuseMap(diffuseMap);
		}
	}

	return model;
}

Ref< Object > MeshEntityReplicator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
	const std::wstring& assetPath,
    const Object* source,
    const model::Model* model,
	const Guid& outputGuid
) const
{
	const MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const MeshComponentData* >(source);

	// Read original mesh asset from source.
	Ref< MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< MeshAsset >(meshComponentData->getMesh());
	if (!meshAsset)
		return nullptr;

	// Create a new mesh asset referencing the modified model.
    Ref< MeshAsset > outputMeshAsset = new MeshAsset();
    outputMeshAsset->setMeshType(MeshAsset::MtStatic);
	outputMeshAsset->setMaterialTemplates(meshAsset->getMaterialTemplates());
	outputMeshAsset->setMaterialShaders(meshAsset->getMaterialShaders());
	outputMeshAsset->setMaterialTextures(meshAsset->getMaterialTextures());
	outputMeshAsset->setTextureSet(meshAsset->getTextureSet());

	// Build output mesh from modified model.
    pipelineBuilder->buildAdHocOutput(
        outputMeshAsset,
        outputGuid,
        model
    );

	return new MeshComponentData(resource::Id< IMesh >(outputGuid));
}

    }
}