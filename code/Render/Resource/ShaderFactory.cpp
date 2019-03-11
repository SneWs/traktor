#include "Database/Instance.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/ShaderResource.h"
#include "Render/Resource/TextureLinker.h"
#include "Render/Resource/TextureProxy.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class TextureReaderAdapter : public TextureLinker::TextureReader
{
public:
	TextureReaderAdapter(resource::IResourceManager* resourceManager)
	:	m_resourceManager(resourceManager)
	{
	}

	virtual Ref< ITexture > read(const Guid& textureGuid)
	{
		resource::Proxy< ITexture > texture;
		if (m_resourceManager->bind(resource::Id< ITexture >(textureGuid), texture))
			return new TextureProxy(texture);
		else
			return (ITexture*)0;
	}

private:
	resource::IResourceManager* m_resourceManager;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderFactory", ShaderFactory, resource::IResourceFactory)

ShaderFactory::ShaderFactory(
	IRenderSystem* renderSystem
)
:	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ShaderFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ShaderResource >();
}

const TypeInfoSet ShaderFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Shader >();
}

bool ShaderFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > ShaderFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< ShaderResource > shaderResource = instance->getObject< ShaderResource >();
	if (!shaderResource)
		return nullptr;

	std::wstring shaderName = instance->getName();
	Ref< Shader > shader = new Shader();

	// Create combination parameter mapping.
	const std::map< std::wstring, uint32_t >& parameterBits = shaderResource->getParameterBits();
	for (std::map< std::wstring, uint32_t >::const_iterator i = parameterBits.begin(); i != parameterBits.end(); ++i)
		shader->m_parameterBits[getParameterHandle(i->first)] = i->second;

	// Create shader techniques.
	const std::vector< ShaderResource::Technique >& techniques = shaderResource->getTechniques();
	for (std::vector< ShaderResource::Technique >::const_iterator i = techniques.begin(); i != techniques.end(); ++i)
	{
		std::wstring programName = shaderName + L"." + i->name;

		Shader::Technique& technique = shader->m_techniques[getParameterHandle(i->name)];
		technique.mask = i->mask;

		for (const auto& resourceCombination : i->combinations)
		{
			if (!resourceCombination.program)
				continue;

			Ref< ProgramResource > programResource = checked_type_cast< ProgramResource* >(resourceCombination.program);
			if (!programResource)
				return nullptr;

			Shader::Combination combination;
			combination.mask = resourceCombination.mask;
			combination.value = resourceCombination.value;
			combination.priority = resourceCombination.priority;
			combination.program = m_renderSystem->createProgram(programResource, programName.c_str());
			if (!combination.program)
				return nullptr;

			// Set implicit texture uniforms.
			TextureReaderAdapter textureReader(resourceManager);
			if (!TextureLinker(textureReader).link(resourceCombination, combination.program))
				return nullptr;

			// Set uniform default values.
			for (const auto& ius : resourceCombination.initializeUniformScalar)
				combination.program->setFloatParameter(getParameterHandle(ius.name), ius.value);
			for (const auto& iuv : resourceCombination.initializeUniformVector)
				combination.program->setVectorParameter(getParameterHandle(iuv.name), iuv.value);

			technique.combinations.push_back(combination);
		}
	}

	shader->setTechnique(getParameterHandle(L"Default"));
	return shader;
}

	}
}
