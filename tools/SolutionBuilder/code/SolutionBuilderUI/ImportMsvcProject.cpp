#include <Ui/FileDialog.h>
#include <Ui/MessageBox.h>
#include <Xml/Document.h>
#include <Xml/Element.h>
#include <Xml/Attribute.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/File.h>
#include <Core/Misc/Split.h>
#include <Core/Misc/String.h>
#include "ImportMsvcProject.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/File.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"ImportMsvcProject", ImportMsvcProject, CustomTool)

namespace
{

	void traverseMsvcProject(Project* project, Filter* filter, xml::Element* group, const Path& vcprojPath, const Path& solutionPath)
	{
		RefArray< xml::Element > files, filters;
		
		group->get(L"File", files);
		for (RefArray< xml::Element >::iterator i = files.begin(); i != files.end(); ++i)
		{
			Ref< xml::Attribute > attribRelativePath = (*i)->getAttribute(L"RelativePath");
			if (!attribRelativePath)
				continue;

			// Transform path relative to vcproj to relative to our solution.
			Path relativePath = attribRelativePath->getValue();
			Path absolutePath = FileSystem::getInstance().getAbsolutePath(vcprojPath, relativePath);
			FileSystem::getInstance().getRelativePath(
				absolutePath,
				solutionPath,
				relativePath
			);

			Ref< ::File > file = gc_new< ::File >();
			file->setFileName(relativePath.getPathName());

			if (filter)
				filter->addItem(file);
			else
				project->addItem(file);
		}

		group->get(L"Filter", filters);
		for (RefArray< xml::Element >::iterator i = filters.begin(); i != filters.end(); ++i)
		{
			Ref< xml::Attribute > attribName = (*i)->getAttribute(L"Name");
			if (!attribName)
				continue;

			std::wstring name = attribName->getValue();

			Ref< Filter > childFilter = gc_new< Filter >();
			childFilter->setName(name);

			if (filter)
				filter->addItem(childFilter);
			else
				project->addItem(childFilter);

			traverseMsvcProject(project, childFilter, *i, vcprojPath, solutionPath);
		}
	}

}

bool ImportMsvcProject::execute(traktor::ui::Widget* parent, Solution* solution, const std::wstring& solutionFileName)
{
	bool result = false;

	ui::FileDialog fileDialog;
	fileDialog.create(parent, L"Import MSVC project", L"MS Visual Studio projects;*.vcproj");
	
	Path filePath;
	if (fileDialog.showModal(filePath))
	{
		xml::Document document;
		if (document.loadFromFile(filePath.getPathName()))
		{
			Ref< xml::Attribute > attribName = document.getDocumentElement()->getAttribute(L"Name");
			std::wstring projectName = attribName ? attribName->getValue() : L"Unnamed";

			Ref< Project > project = gc_new< Project >();
			project->setName(projectName);

			RefArray< xml::Element > configurations;
			document.get(L"/VisualStudioProject/Configurations/Configuration", configurations);
			for (RefArray< xml::Element >::iterator i = configurations.begin(); i != configurations.end(); ++i)
			{
				Ref< xml::Attribute > attribName = (*i)->getAttribute(L"Name");
				std::wstring configurationAndPlatform = attribName ? attribName->getValue() : L"Unnamed|Win32";

				size_t p = configurationAndPlatform.find(L'|');
				if (p == configurationAndPlatform.npos)
					continue;

				std::wstring configurationName = configurationAndPlatform.substr(0, p);
				std::wstring platformName = configurationAndPlatform.substr(p + 1);

				if (compareIgnoreCase(platformName, L"Win32") != 0)
					continue;

				Ref< Configuration > configuration = gc_new< Configuration >();
				configuration->setName(configurationName);

				Ref< xml::Element > elementCompilerTool = (*i)->getSingle(L"Tool[@Name=VCCLCompilerTool]");
				if (elementCompilerTool)
				{
					Ref< xml::Attribute > attribInclude = elementCompilerTool->getAttribute(L"AdditionalIncludeDirectories");
					if (attribInclude)
						Split< std::wstring >::any(attribInclude->getValue(), L";,", configuration->getIncludePaths());

					Ref< xml::Attribute > attribPrep = elementCompilerTool->getAttribute(L"PreprocessorDefinitions");
					if (attribPrep)
						Split< std::wstring >::any(attribPrep->getValue(), L";,", configuration->getDefinitions());

					Ref< xml::Attribute > attribOptimization = elementCompilerTool->getAttribute(L"Optimization");
					if (attribOptimization)
						configuration->setTargetProfile(attribOptimization->getValue() == L"0" ? Configuration::TpDebug : Configuration::TpRelease);
				}

				Ref< xml::Element > elementLinkerTool = (*i)->getSingle(L"Tool[@Name=VCLinkerTool]");
				if (elementLinkerTool)
				{
					Ref< xml::Attribute > attribLibraryPaths = elementLinkerTool->getAttribute(L"AdditionalLibraryDirectories");
					if (attribLibraryPaths)
						Split< std::wstring >::any(attribLibraryPaths->getValue(), L";,", configuration->getLibraryPaths());

					Ref< xml::Attribute > attribLibraries = elementLinkerTool->getAttribute(L"AdditionalDependencies");
					if (attribLibraries)
						Split< std::wstring >::any(attribLibraries->getValue(), L";,", configuration->getLibraries());

					configuration->setTargetFormat(Configuration::TfSharedLibrary);
				}
				else
					configuration->setTargetFormat(Configuration::TfStaticLibrary);

				project->addConfiguration(configuration);
			}

			Ref< xml::Element > filesGroup = document.getSingle(L"/VisualStudioProject/Files");
			if (filesGroup)
			{
				Path vcprojPath = Path(filePath).getPathOnly();
				Path solutionPath = Path(solutionFileName).getPathOnly();
				traverseMsvcProject(project, 0, filesGroup, vcprojPath, solutionPath);
			}

			solution->addProject(project);
			result = true;
		}
		else
			ui::MessageBox::show(parent, L"Unable to open project", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	fileDialog.destroy();
	return result;
}
