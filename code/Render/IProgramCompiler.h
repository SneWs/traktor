/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IProgramCompiler_H
#define traktor_render_IProgramCompiler_H

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class IProgramHints;
class ProgramResource;
class ShaderGraph;

/*! \brief Program compiler interface.
 * \ingroup Render
 */
class T_DLLCLASS IProgramCompiler : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Compiled stats. */
	struct Stats
	{
		uint32_t vertexCost;
		uint32_t pixelCost;
		uint32_t vertexSize;
		uint32_t pixelSize;

		Stats()
		:	vertexCost(0)
		,	pixelCost(0)
		,	vertexSize(0)
		,	pixelSize(0)
		{
		}
	};

	/*! \brief Get platform signature.
	 *
	 * \return Platform signature.
	 */
	virtual const wchar_t* getPlatformSignature() const = 0;

	/*! \brief Compile program.
	 *
	 * \param shaderGraph Program shader graph.
	 * \param settings Compiler settings.
	 * \param name Program name, useful for debugging.
	 * \param optimize Optimization level (0-4; 0 = No optimization; 4 = Maximum optimization).
	 * \param validate Validate compiled program.
	 * \param outStats Optional stats.
	 * \return Compiled program resource.
	 */
	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		int32_t optimize,
		bool validate,
		Stats* outStats
	) const = 0;

	/*! \brief Generate render specific shader if possible.
	 *
	 * \note This is only used to aid optimization of shader graphs
	 * from within the editor.
	 *
	 * \param shaderGraph Program shader graph.
	 * \param settings Compiler settings.
	 * \param name Program name, useful for debugging.
	 * \param optimize Optimization level (0-4; 0 = No optimization; 4 = Maximum optimization).
	 * \param outVertexShader Output render specific shader.
	 * \param outPixelShader Output render specific shader.
	 * \param outComputeShader Output compute specific shader.
	 * \return True if shader was successfully generated.
	 */
	virtual bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		int32_t optimize,
		std::wstring& outVertexShader,
		std::wstring& outPixelShader,
		std::wstring& outComputeShader
	) const = 0;
};

	}
}

#endif	// traktor_render_IProgramCompiler_H
