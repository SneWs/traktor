#include <Core/Io/FileSystem.h>
#include <Core/Io/File.h>
#include <Core/Io/Stream.h>
#include <Core/Io/DynamicMemoryStream.h>
#include <Core/Io/BufferedStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Io/StringReader.h>
#include <Core/Serialization/BinarySerializer.h>
#include <Core/Misc/String.h>
#include <Core/Misc/MD5.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Make/SolutionBuilderMake.h"
#include "SolutionBuilderLIB/Make/DependencyCache.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/File.h"

using namespace traktor;

namespace
{

#define SCAN_RECURSIVE_DEPENDENCIES 0	// Scan header dependencies recursive.

void collectFiles(const Project* project, const RefList< ProjectItem >& items, std::set< Path >& outFiles)
{
	for (RefList< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		if (::File* fileItem = dynamic_type_cast< ::File* >(*i))
			fileItem->getSystemFiles(project->getSourcePath(), outFiles);
		collectFiles(project, (*i)->getItems(), outFiles);
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMake", SolutionBuilderMake, SolutionBuilder)

SolutionBuilderMake::SolutionBuilderMake()
:	m_dialect(MdGnuMake)
,	m_platform(MpMacOSX)
{
}

bool SolutionBuilderMake::create(const CommandLine& cmdLine)
{
	if (cmdLine.hasOption('d'))
	{
		if (cmdLine.getOption('d').getString() == L"nmake")
			m_dialect = MdNMake;
	}

	if (cmdLine.hasOption('p'))
	{
		std::wstring platform = cmdLine.getOption('p').getString();
		if (platform == L"win32")
			m_platform = MpWin32;
		else if (platform == L"osx")
			m_platform = MpMacOSX;
		else if (platform == L"linux")
			m_platform = MpLinux;
		else
			return false;
	}

	if (cmdLine.hasOption('c'))
		m_config = cmdLine.getOption('c').getString();
	else
	{
		m_config = L"$(TRAKTOR_HOME)/bin/make-config-";
		switch (m_platform)
		{
		case MpWin32:
			m_config += L"win32.inc";
			break;
		case MpMacOSX:
			m_config += L"macosx.inc";
			break;
		case MpLinux:
			m_config += L"linux.inc";
			break;
		}
		traktor::log::info << L"Using configuration file \"" << m_config << L"\"" << Endl;
	}

	return true;
}

bool SolutionBuilderMake::generate(Solution* solution)
{
	std::wstring solutionMake = solution->getRootPath() + L"/" + solution->getName() + L".mak";

	if (!FileSystem::getInstance().makeDirectory(solution->getRootPath()))
		return false;

	// Read dependency cache if available.
	{
		Ref< Stream > file = FileSystem::getInstance().open(solution->getRootPath() + L"/DependencyCache.bin", traktor::File::FmRead);
		if (file)
		{
			m_dependencyCache = BinarySerializer(file).readObject< DependencyCache >();
			file->close();
		}
	}
	if (!m_dependencyCache)
	{
		traktor::log::info << L"Creating new dependency cache..." << Endl;
		m_dependencyCache = gc_new< DependencyCache >();
	}

	Ref< Stream > file = FileSystem::getInstance().open(
		solutionMake,
		traktor::File::FmWrite
	);
	if (!file)
		return false;

	FileOutputStream s(file, gc_new< AnsiEncoding >());

	// Sort projects by their dependencies.
	RefList< Project > projects = solution->getProjects();
	RefList< Project > generate;
	while (!projects.empty())
	{
		RefList< Project >::iterator i = projects.begin();
		while (i != projects.end())
		{
			bool satisfied = true;
			const RefList< Dependency >& dependencies = (*i)->getDependencies();
			for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (!is_a< ProjectDependency >(*j))
					continue;

				Project* dependencyProject = static_cast< ProjectDependency* >(*j)->getProject();
				if (std::find(generate.begin(), generate.end(), dependencyProject) == generate.end())
				{
					satisfied = false;
					break;
				}
			}
			if (satisfied)
				break;
			++i;
		}
		generate.push_back(*i);
		projects.erase(i);
	}

	// Collect all types of configurations available in solution.
	std::set< std::wstring > configurationNames;
	for (RefList< Project >::iterator i = generate.begin(); i != generate.end(); ++i)
	{
		const RefList< Configuration >& configurations = (*i)->getConfigurations();
		for (RefList< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
			configurationNames.insert((*j)->getName());
	}

	// Generate "master" makefile.
	s << L"# This makefile is automatically generated, DO NOT EDIT!" << Endl;
	s << Endl;

	s << L".PHONY : All" << Endl;
	s << L"All :" << Endl;
	for (RefList< Project >::iterator i = generate.begin(); i != generate.end(); ++i)
		s << L"\t-$(MAKE) -f " << (*i)->getName() << L"/" << (*i)->getName() << L".mak All" << Endl;
	s << Endl;

	for (std::set< std::wstring >::iterator i = configurationNames.begin(); i != configurationNames.end(); ++i)
	{
		s << L".PHONY : " << *i << Endl;
		s << *i << L" :" << Endl;
		for (RefList< Project >::iterator j = generate.begin(); j != generate.end(); ++j)
		{
			const RefList< Configuration >& configurations = (*j)->getConfigurations();
			for (RefList< Configuration >::const_iterator k = configurations.begin(); k != configurations.end(); ++k)
			{
				if ((*k)->getName() == *i)
				{
					s << L"\t-$(MAKE) -f " << (*j)->getName() << L"/" << (*j)->getName() << L".mak " << *i << Endl;
					break;
				}
			}
		}
		s << Endl;
	}

	s << L".PHONY : Clean" << Endl;
	s << L"Clean :" << Endl;
	for (RefList< Project >::iterator i = generate.begin(); i != generate.end(); ++i)
		s << L"\t-$(MAKE) -f " << (*i)->getName() << L"/" << (*i)->getName() << L".mak Clean" << Endl;
	s << Endl;

	s.close();

	// Generate project makefiles.
	for (RefList< Project >::iterator i = generate.begin(); i != generate.end(); ++i)
	{
		if (!generateProject(solution, *i))
			return false;
	}

	// Write dependency cache.
	if (m_dependencyCache)
	{
		Ref< Stream > file = FileSystem::getInstance().open(solution->getRootPath() + L"/DependencyCache.bin", traktor::File::FmWrite);
		if (file)
		{
			BinarySerializer(file).writeObject(m_dependencyCache);
			file->close();
		}
		else
			traktor::log::warning << L"Unable to save dependency cache" << Endl;
	}

	return true;
}

void SolutionBuilderMake::showOptions() const
{
	traktor::log::info << L"\t-c=[configuration file]	[$(TRAKTOR_HOME)/bin/make-config-(platform).inc]" << Endl;
	traktor::log::info << L"\t-d=[dialect]			[\"gnu\" *, \"nmake\"]" << Endl;
	traktor::log::info << L"\t-p=[platform]			[\"macosx\" *, \"win32\", \"linux\"]" << Endl;
}

bool SolutionBuilderMake::generateProject(Solution* solution, Project* project)
{
	std::set< ::Path > files;
	collectFiles(project, project->getItems(), files);

	RefList< Configuration >& configurations = project->getConfigurations();

	// Create directory for project.
	if (!FileSystem::getInstance().makeDirectory(solution->getRootPath() + L"/" + project->getName()))
		return false;
	
	// Create directory for each configuration.
	for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		Configuration* configuration = *i;
		if (!FileSystem::getInstance().makeDirectory(solution->getRootPath() + L"/" + toLower(configuration->getName())))
			return false;
	}

	std::wstring makeFilePath = solution->getRootPath() + L"/" + project->getName();
	std::wstring makeFile = makeFilePath + L"/" + project->getName() + L".mak";

	traktor::log::info << L"Generating makefile \"" << makeFile << L"\"" << Endl;

	std::vector< uint8_t > buffer;
	buffer.reserve(40000);

	// Generate project file into memory buffer.
	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream s(&bufferStream, gc_new< AnsiEncoding >());

	s << L"# This makefile is automatically generated, DO NOT EDIT!" << Endl;
	s << Endl;
	
	// Include user configuration.
	if (m_dialect == MdNMake)
		s << L"!INCLUDE " << m_config << Endl;
	if (m_dialect == MdGnuMake)
		s << L"include " << m_config << Endl;
	s << Endl;
	
	Path rootPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());

	// Build include strings for each configuration.
	for (RefList< Configuration >::iterator j = configurations.begin(); j != configurations.end(); ++j)
	{
		Configuration* configuration = *j;
		
		s << toUpper(configuration->getName()) + L"_INCLUDE=";

		// Add path to makefile in include search path; needed by resources.
		if (m_platform == MpWin32)
			s << L"/I" << makeFilePath << L" ";
		else if (m_platform == MpMacOSX || m_platform == MpLinux)
			s << L"-I" << makeFilePath << L" ";

		std::vector< std::wstring >& includePaths = configuration->getIncludePaths();
		for (std::vector< std::wstring >::iterator k = includePaths.begin(); k != includePaths.end(); ++k)
		{
			std::wstring configurationIncludePath = *k;
			if (configurationIncludePath.empty())
				continue;
			if (configurationIncludePath[0] != '`')
			{
				Path includePath;
				FileSystem::getInstance().getRelativePath(
					FileSystem::getInstance().getAbsolutePath(configurationIncludePath),
					rootPath,
					includePath
				);
				if (m_platform == MpWin32)
					s << L"/I" << std::wstring(includePath.getPathName()) << L" ";
				else if (m_platform == MpMacOSX || m_platform == MpLinux)
					s << L"-I" << std::wstring(includePath.getPathName()) << L" ";
			}
			else
			{
				// Special include path tool such as pkg-config.
				s << configurationIncludePath << L" ";
			}
		}

		s << Endl;
	}
	s << Endl;

	// Build macro strings for each configuration.
	for (RefList< Configuration >::iterator j = configurations.begin(); j != configurations.end(); ++j)
	{
		Configuration* configuration = *j;
		
		s << toUpper(configuration->getName()) + L"_DEFINES=";

		std::vector< std::wstring >& definitions = configuration->getDefinitions();
		for (std::vector< std::wstring >::iterator k = definitions.begin(); k != definitions.end(); ++k)
		{
			if (k != definitions.begin())
				s << L" ";
			if (m_platform == MpWin32)
				s << L"/D" << *k;
			else if (m_platform == MpMacOSX || m_platform == MpLinux)
				s << L"-D " << *k;
		}

		s << Endl;
	}
	s << Endl;

	// Define the "All" target.
	s << L".PHONY : All" << Endl;
	s << L"All : \\" << Endl;
	for (RefList< Configuration >::iterator j = configurations.begin(); j != configurations.end(); ++j)
	{
		Configuration* configuration = *j;
		s << L"\t" << configuration->getName() << (*j != configurations.back() ? L" \\" : L"") << Endl;
	}
	s << L"\t@echo All" << Endl;
	s << Endl;

	// Define the "Clean" target.
	s << L".PHONY : Clean" << Endl;
	s << L"Clean :" << Endl;
	for (RefList< Configuration >::iterator j = configurations.begin(); j != configurations.end(); ++j)
	{
		Configuration* configuration = *j;
		if (m_platform == MpWin32)
			s << L"\tdel /F /Q " << project->getName() << L"\\\\" << toLower(configuration->getName()) << L"\\\\*.*" << Endl;
		else if (m_platform == MpMacOSX || m_platform == MpLinux)
			s << L"\trm " << project->getName() << L"/" << toLower(configuration->getName()) << L"/*.*" << Endl;
	}
	s << Endl;

	// Define build rules for each target.
	for (RefList< Configuration >::iterator j = configurations.begin(); j != configurations.end(); ++j)
	{
		Configuration* configuration = *j;

		s << L"# " << configuration->getName() << Endl;
		s << Endl;

		s << L".PHONY : " << configuration->getName() << Endl;
		s << configuration->getName() << L" : \\" << Endl;

		// Define target's object dependencies.
		bool firstTarget = true;
		uint32_t targetCount = 0;

		for (std::set< ::Path >::iterator k = files.begin(); k != files.end(); ++k)
		{
			std::wstring extension = toLower(k->getExtension());
			if (extension == L"png")	// @fixme Custom build rules
			{
				std::wstring fileName = k->getFileName();
				std::wstring fileNameNoExt = k->getFileNameNoExtension();

				if (!firstTarget)
					s << L" \\" << Endl;

				s << L"\t" << project->getName() << L"/Resources/" << fileNameNoExt << L".h";
			
				firstTarget = false;
			}
			else if (extension == L"c" || extension == L"cc" || extension == L"cpp" || extension == L"mm")
			{
				std::wstring fileName = k->getFileName();
				std::wstring fileNameNoExt = k->getFileNameNoExtension();

				if (!firstTarget)
					s << L" \\" << Endl;

				if (m_platform == MpWin32)
					s << L"\t" << project->getName() << L"/" << configuration->getName() << L"/" << fileNameNoExt << L"_" << targetCount << L".obj";
				else if (m_platform == MpMacOSX || m_platform == MpLinux)
					s << L"\t" << project->getName() << L"/" << configuration->getName() << L"/" << fileNameNoExt << L"_" << targetCount << L".o";

				firstTarget = false;
				targetCount++;
			}
		}
		s << Endl;

		std::wstring productSuffix = configuration->getTargetProfile() == Configuration::TpDebug ? L"_d" : L"";

		// Define target rule depending on format.
		if (configuration->getTargetFormat() == Configuration::TfStaticLibrary)
		{
			if (m_platform == MpWin32)
				s << L"\t$(AR) $** /OUT:" << toLower(configuration->getName()) << L"/" << project->getName() << productSuffix << L".lib" << Endl;
			else if (m_platform == MpMacOSX)
				s << L"\t$(AR) -static -o " << toLower(configuration->getName()) << L"/lib" << project->getName() << productSuffix << L".a $^" << Endl;
			else if (m_platform == MpLinux)
				s << L"\t$(AR) rcs " << toLower(configuration->getName()) << L"/lib" << project->getName() << productSuffix << L".a $^" << Endl;
		}
		else	// Shared or executable, perform actual linkage.
		{
			// Collect linking dependencies.
			std::set< std::wstring > libraryPaths;
			std::vector< std::wstring > libraryNames;

			collectLinkDependencies(
				solution,
				project,
				configuration->getName(),
				libraryPaths,
				libraryNames
			);

			// Build linking string.
			std::wstring libPaths = L"";
			std::wstring libs = L" $(LINK_LIBS)";

			for (std::set< std::wstring >::iterator it = libraryPaths.begin(); it != libraryPaths.end(); ++it)
			{
				if (m_platform == MpWin32)
					libPaths += L" /LIBPATH:" + *it;
				else
					libPaths += L" -L" + *it;
			}

			for (std::vector< std::wstring >::iterator it = libraryNames.begin(); it != libraryNames.end(); ++it)
			{
				if (it->empty())
					continue;

				if ((*it)[0] != '`')
				{
					if (m_platform == MpWin32)
						libs += L" " + *it;
					else
					{
						if (endsWith(*it, L".framework"))
							libs += L" -framework " + it->substr(0, it->length() - 10);
						else
							libs += L" -l" + *it;
					}
				}
				else
					libs += L" " + *it;
			}

			// Write link rule.
			if (m_platform == MpWin32)
			{
				if (configuration->getTargetFormat() == Configuration::TfSharedLibrary)
					s << L"\t$(LINK) $** " << libs << L" /DLL /MACHINE:X86 /SUBSYSTEM:WINDOWS $(ADDITIONAL_LIBRARY_PATHS) /OUT:" << toLower(configuration->getName()) << L"/" << project->getName() << productSuffix << L".dll" << Endl;
				else if (configuration->getTargetFormat() == Configuration::TfExecutable)
					s << L"\t$(LINK) $** " << libs << L" /MACHINE:X86 /SUBSYSTEM:WINDOWS $(ADDITIONAL_LIBRARY_PATHS) /OUT:" << toLower(configuration->getName()) << L"/" << project->getName() << productSuffix << L".exe" << Endl;
				else if (configuration->getTargetFormat() == Configuration::TfExecutableConsole)
					s << L"\t$(LINK) $** " << libs << L" /MACHINE:X86 /SUBSYSTEM:WINDOWS $(ADDITIONAL_LIBRARY_PATHS) /OUT:" << toLower(configuration->getName()) << L"/" << project->getName() << productSuffix << L".exe" << Endl;
			}
			else if (m_platform == MpMacOSX)
			{
				if (configuration->getTargetFormat() == Configuration::TfSharedLibrary)
				{
					std::wstring productName = L"lib" + project->getName() + productSuffix + L".dylib";
					s << L"\t$(LINK) $^ -dynamiclib -install_name @executable_path/" << productName << libPaths << libs << L" -o " << toLower(configuration->getName()) << L"/" << productName << Endl;
				}
				else if (configuration->getTargetFormat() == Configuration::TfExecutable)
					s << L"\t$(LINK) $^" << libPaths << libs << L" -o " << toLower(configuration->getName()) << L"/" << project->getName() << productSuffix << Endl;
				else if (configuration->getTargetFormat() == Configuration::TfExecutableConsole)
					s << L"\t$(LINK) $^" << libPaths << libs << L" -o " << toLower(configuration->getName()) << L"/" << project->getName() << productSuffix << Endl;
			}
			else if (m_platform == MpLinux)
			{
				if (configuration->getTargetFormat() == Configuration::TfSharedLibrary)
				{
					std::wstring productName = L"lib" + project->getName() + productSuffix;
					s << L"\t$(LINK) -shared -Wl,-soname," << productName << L".so -o " << toLower(configuration->getName()) << L"/" << productName << L".so $^" << libPaths << libs << Endl;
				}
				else if (configuration->getTargetFormat() == Configuration::TfExecutable)
					s << L"\t$(LINK) $^" << libPaths << libs << L" -o " << toLower(configuration->getName()) << L"/" << project->getName() << productSuffix << Endl;
				else if (configuration->getTargetFormat() == Configuration::TfExecutableConsole)
					s << L"\t$(LINK) $^" << libPaths << libs << L" -o " << toLower(configuration->getName()) << L"/" << project->getName() << productSuffix << Endl;
			}
		}
		s << Endl;

		if (!FileSystem::getInstance().makeDirectory(solution->getRootPath() + L"/" + project->getName() + L"/" + configuration->getName()))
			return false;

		// Create custom build rules.
		// @fixme Currently hardcoded for png resources with BinaryInclude tool.
		for (std::set< ::Path >::iterator k = files.begin(); k != files.end(); ++k)
		{
			std::wstring extension = toLower(k->getExtension());
			if (extension != L"png")
				continue;

			Path absoluteRootPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());
			Path absoluteFilePath = FileSystem::getInstance().getAbsolutePath(k->normalized());

			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				absoluteFilePath,
				absoluteRootPath,
				relativePath
			);

			if (relativePath.getPathName().empty())
			{
				traktor::log::warning << L"Unable to add file \"" << k->getPathName() << L"\", unable to resolve relative path from \"" << absoluteRootPath.getPathName() << L"\" to \"" << absoluteFilePath.getPathName() << L"\"" << Endl;
				continue;
			}

			std::wstring fileName = relativePath.getFileName();
			std::wstring fileNameNoExt = relativePath.getFileNameNoExtension();

			s << project->getName() << L"/Resources/" << fileNameNoExt << L".h : \\" << Endl;
			s << L"\t" << relativePath.getPathName() << Endl;
			s << L"\t$(BINARY_INCLUDE) " << relativePath.getPathName() << L" $@ c_Resource" << fileNameNoExt << Endl;
			s << Endl;
		}

		// Create compile rules.
		targetCount = 0;
		for (std::set< ::Path >::iterator k = files.begin(); k != files.end(); ++k)
		{
			std::wstring extension = toLower(k->getExtension());
			if (extension != L"c" && extension != L"cc" && extension != L"cpp" && extension != L"mm")
				continue;

			Path absoluteRootPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());
			Path absoluteFilePath = FileSystem::getInstance().getAbsolutePath(k->normalized());

			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				absoluteFilePath,
				absoluteRootPath,
				relativePath
			);

			if (relativePath.getPathName().empty())
			{
				traktor::log::warning << L"Unable to add file \"" << k->getPathName() << L"\", unable to resolve relative path from \"" << absoluteRootPath.getPathName() << L"\" to \"" << absoluteFilePath.getPathName() << L"\"" << Endl;
				continue;
			}

			std::set< std::wstring > resolvedDependencies;
			scanDependencies(
				solution,
				configuration,
				k->getPathName(),
				resolvedDependencies
			);

			std::wstring fileName = relativePath.getFileName();
			std::wstring fileNameNoExt = relativePath.getFileNameNoExtension();

			if (m_platform == MpWin32)
				s << project->getName() << L"/" << configuration->getName() << L"/" << fileNameNoExt << L"_" << targetCount << L".obj : \\" << Endl;
			else if (m_platform == MpMacOSX || m_platform == MpLinux)
				s << project->getName() << L"/" << configuration->getName() << L"/" << fileNameNoExt << L"_" << targetCount << L".o : \\" << Endl;
			s << L"\t" << relativePath.getPathName();

			for (std::set< std::wstring >::iterator j = resolvedDependencies.begin(); j != resolvedDependencies.end(); ++j)
				s << L" \\" << Endl << L"\t" << *j;
			
			s << Endl;

			std::wstring profile = (extension != L"mm") ? L"$(CC_FLAGS" : L"$(MM_FLAGS";
			if (configuration->getTargetProfile() == Configuration::TpDebug)
				profile += L"_DEBUG";
			else if (configuration->getTargetProfile() == Configuration::TpRelease)
				profile += L"_RELEASE";
			if (configuration->getTargetFormat() == Configuration::TfSharedLibrary)
				profile += L"_SHARED";
			else if (configuration->getTargetFormat() == Configuration::TfStaticLibrary)
				profile += L"_STATIC";
			else
				profile += L"_EXECUTABLE";
			profile += L")";

			if (m_platform == MpWin32)
				s << L"\t$(CC) " << profile << L" $(" << toUpper(configuration->getName()) << L"_INCLUDE) $(" << toUpper(configuration->getName()) << L"_DEFINES) " << relativePath.getPathName() << L" /Fo$@" << Endl;
			else if (m_platform == MpMacOSX)
			{
				if (extension == L"mm")
					s << L"\t$(MM) -c " << profile << L" $(" << toUpper(configuration->getName()) << L"_INCLUDE) $(" << toUpper(configuration->getName()) << L"_DEFINES) " << relativePath.getPathName() << L" -o $@" << Endl;
				else
					s << L"\t$(CC) -c " << profile << L" $(" << toUpper(configuration->getName()) << L"_INCLUDE) $(" << toUpper(configuration->getName()) << L"_DEFINES) " << relativePath.getPathName() << L" -o $@" << Endl;
			}
			else if (m_platform == MpLinux)
				s << L"\t$(CC) -c " << profile << L" $(" << toUpper(configuration->getName()) << L"_INCLUDE) $(" << toUpper(configuration->getName()) << L"_DEFINES) " << relativePath.getPathName() << L" -o $@" << Endl;
			s << Endl;

			targetCount++;
		}
	}

	s.close();

	if (!buffer.empty())
	{
		Ref< Stream > file = FileSystem::getInstance().open(
			makeFile,
			traktor::File::FmWrite
		);
		if (!file)
			return false;

		file->write(&buffer[0], int(buffer.size()));
		file->close();
	}

	return true;
}

void SolutionBuilderMake::collectLinkDependencies(
	Solution* buildSolution,
	Project* project,
	const std::wstring& configurationName,
	std::set< std::wstring >& outLibraryPaths,
	std::vector< std::wstring >& outLibraryNames
)
{
	// Add 3rd-party libraries.
	Ref< Configuration > configuration = project->getConfiguration(configurationName);
	if (configuration)
	{
		const std::vector< std::wstring >& externalLibraryPaths = configuration->getLibraryPaths();
		const std::vector< std::wstring >& externalLibraryNames = configuration->getLibraries();

		for (std::vector< std::wstring >::const_iterator j = externalLibraryPaths.begin(); j != externalLibraryPaths.end(); ++j)
			outLibraryPaths.insert(*j);

		for (std::vector< std::wstring >::const_iterator j = externalLibraryNames.begin(); j != externalLibraryNames.end(); ++j)
		{
			std::vector< std::wstring >::iterator it = std::find(outLibraryNames.begin(), outLibraryNames.end(), *j);
			if (it != outLibraryNames.end())
				outLibraryNames.erase(it);
			outLibraryNames.push_back(*j);
		}
	}

	// Add products from project's dependencies.
	const RefList< Dependency >& dependencies = project->getDependencies();
	for (RefList< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< Solution > dependentSolution;
		Ref< Project > dependentProject;

		if (is_a< ProjectDependency >(*i))
		{
			dependentSolution = buildSolution;
			dependentProject = static_cast< ProjectDependency* >(*i)->getProject();
		}
		else if (is_a< ExternalDependency >(*i))
		{
			dependentSolution = static_cast< ExternalDependency* >(*i)->getSolution();
			dependentProject = static_cast< ExternalDependency* >(*i)->getProject();
		}

		T_ASSERT (dependentSolution);
		T_ASSERT (dependentProject);

		Ref< Configuration > configuration = dependentProject->getConfiguration(configurationName);
		if (configuration)
		{
			// Add library paths.
			Path libraryPath = dependentSolution->getRootPath() + L"/" + toLower(configuration->getName());

			// Ensure both paths are absolute.
			libraryPath = FileSystem::getInstance().getAbsolutePath(libraryPath);
			Path rootPath = FileSystem::getInstance().getAbsolutePath(buildSolution->getRootPath());

			// Create relative library path.
			Path libraryPathRelative;
			if (!FileSystem::getInstance().getRelativePath(libraryPath, rootPath, libraryPathRelative))
			{
				traktor::log::warning << L"Unable to construct relative path to \"" << libraryPath.getPathName() << L"\"" << Endl;
				continue;
			}

			outLibraryPaths.insert(libraryPathRelative.getPathName());

			// Add libraries.
			std::wstring librarySuffix = configuration->getTargetProfile() == Configuration::TpDebug ? L"_d" : L"";
			if (m_platform == MpWin32)
				librarySuffix += L".lib";

			std::wstring libraryName = dependentProject->getName() + librarySuffix;
			std::vector< std::wstring >::iterator it = std::find(outLibraryNames.begin(), outLibraryNames.end(), libraryName);
			if (it != outLibraryNames.end())
				outLibraryNames.erase(it);
			outLibraryNames.push_back(libraryName);
		}

		collectLinkDependencies(buildSolution, dependentProject, configurationName, outLibraryPaths, outLibraryNames);
	}
}

bool SolutionBuilderMake::scanDependencies(
	Solution* solution,
	Configuration* configuration,
	const std::wstring& fileName,
	std::set< std::wstring >& outDependencies
)
{
	std::wstring key = fileName + L"_" + configuration->getName();

	// Calculate MD5 hash of source file.
	Ref< Stream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return false;

	MD5 md5;
	md5.begin();
	while (file->available() > 0)
	{
		uint8_t buf[1024];
		uint32_t nread = file->read(buf, sizeof(buf));
		md5.feed(buf, nread);
	}
	md5.end();

	file->close();

	if (m_dependencyCache->get(key, md5, outDependencies))
		return true;

	// Not in dependency cache, need to manually resolve all dependencies.
	std::set< std::wstring > visitiedDependencies;
	if (!scanDependencies(solution, configuration, fileName, visitiedDependencies, outDependencies))
		return false;

	// Add dependencies to cache.
	m_dependencyCache->set(key, md5, outDependencies);

	return true;
}

bool SolutionBuilderMake::scanDependencies(
	Solution* solution,
	Configuration* configuration,
	const std::wstring& fileName,
	std::set< std::wstring >& visitedDependencies,
	std::set< std::wstring >& resolvedDependencies
)
{
	Ref< Stream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return false;

	BufferedStream bufferedFile(file);
	StringReader sr(&bufferedFile, gc_new< AnsiEncoding >());

	std::wstring line;
	while (sr.readLine(line) >= 0)
	{
		std::wstring::size_type p = line.find(L"#include ");
		if (p == line.npos)
			continue;

		std::wstring dep = trim(line.substr(p + 9));
		if (dep.length() <= 2 || dep[0] != L'\"')
			continue;

		dep = dep.substr(1);

		p = dep.find_first_of('\"');
		if (p == line.npos)
			continue;

		dep = dep.substr(0, p);

		if (visitedDependencies.find(dep) != visitedDependencies.end())
			continue;

		visitedDependencies.insert(dep);

		std::vector< std::wstring >& includePaths = configuration->getIncludePaths();
		for (std::vector< std::wstring >::iterator i = includePaths.begin(); i != includePaths.end(); ++i)
		{
			std::wstring dependencyName = (*i) + L"/" + dep;

#if SCAN_RECURSIVE_DEPENDENCIES
			if (!scanDependencies(
				solution,
				configuration,
				dependencyName,
				visitedDependencies,
				resolvedDependencies
			))
				continue;
#endif

			Path relativeDependencyName;
			FileSystem::getInstance().getRelativePath(
				dependencyName,
				solution->getRootPath(),
				relativeDependencyName
			);
			if (relativeDependencyName.getPathName().empty())
				continue;

			resolvedDependencies.insert(relativeDependencyName.getPathName());
			break;
		}
	}

	file->close();
	return true;
}

