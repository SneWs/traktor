#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class Edit;
class ListBox;

	}

	namespace sb
	{

class Project;
class Solution;

class ExtractSolutionDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, Solution* solution);

	void getSelectedProjects(RefArray< Project >& outProjects) const;

	std::wstring getSolutionFile() const;

	std::wstring getSolutionName() const;

private:
	Ref< ui::ListBox > m_listProjects;
	Ref< ui::Edit > m_editSolutionFile;
	Ref< ui::Edit > m_editSolutionName;
};

	}
}

