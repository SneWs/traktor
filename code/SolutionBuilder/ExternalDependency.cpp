#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/SolutionLoader.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ExternalDependency", 3, ExternalDependency, Dependency)

ExternalDependency::ExternalDependency(const std::wstring& solutionFileName, const std::wstring& projectName)
:	m_solutionFileName(solutionFileName)
,	m_projectName(projectName)
{
}

void ExternalDependency::setSolutionFileName(const std::wstring& solutionFileName)
{
	m_solutionFileName = solutionFileName;
}

const std::wstring& ExternalDependency::getSolutionFileName() const
{
	return m_solutionFileName;
}

Solution* ExternalDependency::getSolution() const
{
	return m_solution;
}

Project* ExternalDependency::getProject() const
{
	return m_project;
}

std::wstring ExternalDependency::getName() const
{
	return m_projectName;
}

std::wstring ExternalDependency::getLocation() const
{
	return m_solutionFileName;
}

bool ExternalDependency::resolve(const Path& referringSolutionPath, SolutionLoader* solutionLoader)
{
	if (m_solution && m_project)
		return true;

	Path solutionFileName = Path(referringSolutionPath.getPathOnly()) + Path(m_solutionFileName);

	m_solution = solutionLoader->load(solutionFileName.getPathName());
	if (!m_solution)
	{
		log::error << L"Unable to load external solution \"" << solutionFileName.getPathName() << L"\"; corrupt or missing" << Endl;
		return false;
	}

	const RefArray< Project >& projects = m_solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		if ((*i)->getName() == m_projectName)
		{
			m_project = *i;
			break;
		}
	}

	if (!m_project)
	{
		log::error << L"Unable to resolve external dependency \"" << m_projectName << L"\"; no such project in solution \"" << solutionFileName.getPathName() << L"\"" << Endl;
		return false;
	}

	// Resolve other external dependencies from this dependency.
	RefArray< Dependency > originalDependencies = m_project->getDependencies();
	RefArray< Dependency > resolvedDependencies;

	for (RefArray< Dependency >::const_iterator i = originalDependencies.begin(); i != originalDependencies.end(); ++i)
	{
		if (ProjectDependency* projectDependency = dynamic_type_cast< ProjectDependency* >(*i))
		{
			Ref< ExternalDependency > externalDependency = new ExternalDependency(
				m_solutionFileName,
				projectDependency->getProject()->getName()
			);
			if (externalDependency->resolve(solutionFileName, solutionLoader))
				resolvedDependencies.push_back(externalDependency);
			else
				return false;
		}
		else if (ExternalDependency* externalDependency = dynamic_type_cast< ExternalDependency* >(*i))
		{
			if (externalDependency->resolve(solutionFileName, solutionLoader))
				resolvedDependencies.push_back(externalDependency);
			else
				return false;
		}
	}

	// Replace dependencies with resolved dependencies.
	m_project->setDependencies(resolvedDependencies);

	return true;
}

void ExternalDependency::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		Dependency::serialize(s);

	s >> Member< std::wstring >(L"solutionFileName", m_solutionFileName);
	s >> Member< std::wstring >(L"projectName", m_projectName);
}

	}
}
