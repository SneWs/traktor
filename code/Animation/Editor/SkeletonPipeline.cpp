#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/Editor/SkeletonPipeline.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Transform.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.SkeletonPipeline", 5, SkeletonPipeline, editor::IPipeline)

bool SkeletonPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void SkeletonPipeline::destroy()
{
}

TypeInfoSet SkeletonPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< SkeletonAsset >();
}

uint32_t SkeletonPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool SkeletonPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const SkeletonAsset > skeletonAsset = checked_type_cast< const SkeletonAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), skeletonAsset->getFileName().getOriginal());
	return true;
}

bool SkeletonPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< const SkeletonAsset > skeletonAsset = checked_type_cast< const SkeletonAsset* >(sourceAsset);

	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + skeletonAsset->getFileName());
	Ref< model::Model > model = model::ModelFormat::readAny(filePath);
	if (!model)
	{
		log::error << L"Unable to build skeleton; no such file \"" << skeletonAsset->getFileName().getPathName() << L"\"." << Endl;
		return false;
	}

	// Scale model according to scale factor in asset.
	model::Transform(scale(skeletonAsset->getScale(), skeletonAsset->getScale(), skeletonAsset->getScale())).apply(*model);

	Ref< Skeleton > skeleton = new Skeleton();

	for (const auto& modelJoint : model->getJoints())
	{
		Ref< Joint > joint = new Joint();

		if (modelJoint.getParent() != model::c_InvalidIndex)
			joint->setParent(modelJoint.getParent());

		joint->setName(modelJoint.getName());
		joint->setTransform(modelJoint.getTransform());
		joint->setRadius(skeletonAsset->getRadius());

		skeleton->addJoint(joint);
	}

	if (!skeleton)
	{
		log::error << L"Unable to build skeleton; import failed." << Endl;
		return false;
	}

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!instance)
	{
		log::error << L"Unable to build skeleton; unable to create output instance." << Endl;
		return false;
	}

	instance->setObject(skeleton);

	if (!instance->commit())
	{
		log::error << L"Unable to build skeleton; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > SkeletonPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

	}
}
