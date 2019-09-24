#include "Render/Capture/ProgramResourceCapture.h"
#include "Render/Capture/Editor/ProgramCompilerCapture.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramCompilerCapture", ProgramCompilerCapture, IProgramCompiler)

ProgramCompilerCapture::ProgramCompilerCapture(IProgramCompiler* compiler)
:	m_compiler(compiler)
{
}

const wchar_t* ProgramCompilerCapture::getRendererSignature() const
{
	return m_compiler->getRendererSignature();
}

Ref< ProgramResource > ProgramCompilerCapture::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	Ref< ProgramResource > resource = m_compiler->compile(shaderGraph, settings, name, optimize, validate, outStats);
	if (!resource)
		return 0;

	Ref< ProgramResourceCapture > resourceCapture = new ProgramResourceCapture();
	resourceCapture->m_embedded = resource;

	// Record all uniforms used in shader.
	// shaderGraph->findNodesOf< Uniform >(resourceCapture->m_uniforms);
	// shaderGraph->findNodesOf< IndexedUniform >(resourceCapture->m_indexedUniforms);

	// Keep copy of readable shader in capture.
	m_compiler->generate(
		shaderGraph,
		settings,
		name,
		optimize,
		resourceCapture->m_vertexShader,
		resourceCapture->m_pixelShader,
		resourceCapture->m_computeShader
	);

	return resourceCapture;
}

bool ProgramCompilerCapture::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	int32_t optimize,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader,
	std::wstring& outComputeShader
) const
{
	return m_compiler->generate(shaderGraph, settings, name, optimize, outVertexShader, outPixelShader, outComputeShader);
}

	}
}
