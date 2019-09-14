#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "World/EntityData.h"
#include "World/EntityEventSetData.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEventData.h"
#include "World/Editor/EntityPipeline.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityPipeline", 1, EntityPipeline, editor::IPipeline)

bool EntityPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void EntityPipeline::destroy()
{
}

TypeInfoSet EntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< EntityData >();
	typeSet.insert< IEntityEventData >();
	return typeSet;
}

bool EntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< Reflection > reflection = Reflection::create(sourceAsset);
	if (!reflection)
		return false;

	// Find all members which reference child entities.
	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< const RfmObject > objectMember = checked_type_cast< const RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const EntityData* entityData = dynamic_type_cast< const EntityData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityData);
		}
		else if (const IEntityEventData* entityEventData = dynamic_type_cast< const IEntityEventData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityEventData);
		}
		else if (const IEntityComponentData* entityComponentData = dynamic_type_cast< const IEntityComponentData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityComponentData);
		}
		else if (const EntityEventSetData* entityEventSetData = dynamic_type_cast< const EntityEventSetData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityEventSetData);
		}
		else if (objectMember->get())
		{
			Ref< Reflection > childReflection = Reflection::create(objectMember->get());
			if (childReflection)
				childReflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
		}
	}

	return true;
}

bool EntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< ISerializable > outputAsset = pipelineBuilder->buildOutput(sourceInstance, sourceAsset, buildParams);
	if (!outputAsset)
		return false;

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
		return false;

	outputInstance->setObject(outputAsset);

	if (!outputInstance->commit())
		return false;

	return true;
}

Ref< ISerializable > EntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	Ref< Reflection > reflection = Reflection::create(sourceAsset);
	if (!reflection)
		return nullptr;

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const EntityData* entityData = dynamic_type_cast< const EntityData* >(objectMember->get()))
		{
			objectMember->set(pipelineBuilder->buildOutput(sourceInstance, entityData));
		}
		else if (const IEntityComponentData* entityComponentData = dynamic_type_cast< const IEntityComponentData* >(objectMember->get()))
		{
			objectMember->set(pipelineBuilder->buildOutput(sourceInstance, entityComponentData));
		}
		else if (const IEntityEventData* entityEventData = dynamic_type_cast< const IEntityEventData* >(objectMember->get()))
		{
			objectMember->set(pipelineBuilder->buildOutput(sourceInstance, entityEventData));
		}
		else if (objectMember->get())
		{
			Ref< Reflection > childReflection = Reflection::create(objectMember->get());
			if (childReflection)
				childReflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
		}
	}

	return reflection->clone();
}

	}
}
