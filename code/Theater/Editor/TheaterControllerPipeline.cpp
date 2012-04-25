#include "Core/Serialization/DeepClone.h"
#include "Editor/IPipelineBuilder.h"
#include "Theater/TheaterControllerData.h"
#include "Theater/TrackData.h"
#include "Theater/Editor/TheaterControllerPipeline.h"
#include "World/Entity/EntityData.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterControllerPipeline", 0, TheaterControllerPipeline, editor::IPipeline)

bool TheaterControllerPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void TheaterControllerPipeline::destroy()
{
}

TypeInfoSet TheaterControllerPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TheaterControllerData >());
	return typeSet;
}

bool TheaterControllerPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	return true;
}

bool TheaterControllerPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	return false;
}

Ref< ISerializable > TheaterControllerPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	const TheaterControllerData* sourceControllerData = checked_type_cast< const TheaterControllerData*, false >(sourceAsset);
	
	Ref< TheaterControllerData > controllerData = new TheaterControllerData(*sourceControllerData);
	if (!controllerData)
		return 0;

	RefArray< TrackData >& trackData = controllerData->getTrackData();
	for (uint32_t i = 0; i < trackData.size(); ++i)
	{
		Ref< world::EntityData > entityData = checked_type_cast< world::EntityData* >(pipelineBuilder->buildOutput(trackData[i]->getEntityData()));
		Ref< world::EntityData > lookAtEntityData = checked_type_cast< world::EntityData* >(pipelineBuilder->buildOutput(trackData[i]->getLookAtEntityData()));

		trackData[i] = new TrackData(*trackData[i]);
		trackData[i]->setEntityData(entityData);
		trackData[i]->setLookAtEntityData(lookAtEntityData);
	}

	return controllerData;
}

	}
}
