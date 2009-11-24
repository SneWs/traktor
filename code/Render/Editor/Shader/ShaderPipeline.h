#ifndef traktor_render_ShaderPipeline_H
#define traktor_render_ShaderPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IProgramCompiler;

class T_DLLCLASS ShaderPipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	ShaderPipeline();

	virtual bool create(const editor::IPipelineSettings* settings);

	virtual void destroy();

	virtual TypeInfoSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const Object* buildParams,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t reason
	) const;

private:
	Ref< IProgramCompiler > m_programCompiler;
	int32_t m_optimize;
	bool m_validate;
	bool m_debugCompleteGraphs;
	std::wstring m_debugPath;
};

	}
}

#endif	// traktor_render_ShaderPipeline_H
