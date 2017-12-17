/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXCustomBuildTool_H
#define SolutionBuilderMsvcVCXCustomBuildTool_H

#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXBuildTool.h"

namespace traktor
{
	namespace sb
	{

class SolutionBuilderMsvcVCXCustomBuildTool : public SolutionBuilderMsvcVCXBuildTool
{
	T_RTTI_CLASS;

public:
	virtual bool generateProject(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const std::wstring& filter,
		const Path& fileName,
		OutputStream& os
	) const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_command;
	std::wstring m_message;
	std::wstring m_outputs;
};

	}
}

#endif	// SolutionBuilderMsvcVCXCustomBuildTool_H
