#include <sstream>
#include <Ui/Application.h>
#include <Ui/MessageBox.h>
#include <Ui/FloodLayout.h>
#include <Ui/FileDialog.h>
#include <Ui/MethodHandler.h>
#include <Ui/Bitmap.h>
#include <Ui/Custom/Splitter.h>
#include <Ui/Events/MouseEvent.h>
#include <Ui/Events/CommandEvent.h>
#include <Ui/Events/CloseEvent.h>
#include <Drawing/Formats/ImageFormatBmp.h>
#include <Xml/XmlSerializer.h>
#include <Xml/XmlDeserializer.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/MemoryStream.h>
#include <Core/Serialization/DeepHash.h>
#include <Core/Serialization/DeepClone.h>
#include <Core/Log/Log.h>
#include "SolutionForm.h"
#include "SolutionPropertyPage.h"
#include "ProjectPropertyPage.h"
#include "ConfigurationPropertyPage.h"
#include "MRU.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/File.h"

#include "AddMultipleConfigurations.h"
#include "ImportProject.h"
#include "EditConfigurations.h"
#include "ImportMsvcProject.h"

// Embedded resources.
#include "SolutionBitmap.h"
#include "TraktorBitmap.h"

using namespace traktor;

#define TITLE L"SolutionBuilder v1.99.8"

T_IMPLEMENT_RTTI_CLASS(L"SolutionForm", SolutionForm, ui::Form)

namespace
{

	struct ProjectSortPredicate
	{
		bool operator () (const Project* p1, const Project* p2) const
		{
			return p1->getName().compare(p2->getName()) < 0;
		}
	};

}

bool SolutionForm::create(const traktor::CommandLine& cmdLine)
{
	if (!ui::Form::create(
		TITLE,
		800,
		600,
		ui::Form::WsDefault,
		gc_new< ui::FloodLayout >()
	))
		return false;

	setIcon(ui::Bitmap::load(c_traktorBitmap, sizeof(c_traktorBitmap), L"bmp"));

	addTimerEventHandler(ui::createMethodHandler(this, &SolutionForm::eventTimer));
	addCloseEventHandler(ui::createMethodHandler(this, &SolutionForm::eventClose));

	m_shortcutTable = gc_new< ui::ShortcutTable >();
	m_shortcutTable->create();
	m_shortcutTable->addCommand(ui::KsControl, 'N', ui::Command(L"File.New"));
	m_shortcutTable->addCommand(ui::KsControl, 'O', ui::Command(L"File.Open"));
	m_shortcutTable->addCommand(ui::KsControl, 'S', ui::Command(L"File.Save"));
	m_shortcutTable->addCommand(ui::KsControl | ui::KsShift, 'S', ui::Command(L"File.SaveAs"));
	m_shortcutTable->addCommand(ui::KsControl, 'X', ui::Command(L"File.Exit"));
	m_shortcutTable->addShortcutEventHandler(ui::createMethodHandler(this, &SolutionForm::eventShortcut));

	m_menuBar = gc_new< ui::MenuBar >();
	m_menuBar->create(this);
	m_menuBar->addClickEventHandler(ui::createMethodHandler(this, &SolutionForm::eventMenuClick));

	m_menuItemMRU = gc_new< ui::MenuItem >(L"Recent");

	Ref< ui::MenuItem > menuFile = gc_new< ui::MenuItem >(L"File");
	menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"File.New"), L"&New"));
	menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"File.Open"), L"&Open..."));
	menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"File.Save"), L"&Save"));
	menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"File.SaveAs"), L"Save As..."));
	menuFile->add(m_menuItemMRU);
	menuFile->add(gc_new< ui::MenuItem >(L"-"));
	menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"File.Exit"), L"E&xit"));
	m_menuBar->add(menuFile);

	Ref< ui::MenuItem > menuTools = gc_new< ui::MenuItem >(L"Tools");
	menuTools->add(gc_new< ui::MenuItem >(ui::Command(L"Tools.AddMultipleConfigurations"), L"&Add multiple configurations..."));
	menuTools->add(gc_new< ui::MenuItem >(ui::Command(L"Tools.EditConfigurations"), L"&Edit configurations..."));
	menuTools->add(gc_new< ui::MenuItem >(ui::Command(L"Tools.ImportProject"), L"&Import project..."));
	menuTools->add(gc_new< ui::MenuItem >(ui::Command(L"Tools.ImportMsvcProject"), L"&Import MSVC project..."));
	m_menuBar->add(menuTools);

	Ref< ui::custom::Splitter > splitter = gc_new< ui::custom::Splitter >();
	splitter->create(this, true, 300);

	m_treeSolution = gc_new< ui::TreeView >();
	m_treeSolution->create(
		splitter,
		ui::WsClientBorder |
		ui::TreeView::WsAutoEdit |
		ui::TreeView::WsTreeButtons |
		ui::TreeView::WsTreeLines
	);
	m_treeSolution->addImage(ui::Bitmap::load(c_solutionBitmap, sizeof(c_solutionBitmap), L"bmp"), 6);
	m_treeSolution->addButtonDownEventHandler(ui::createMethodHandler(this, &SolutionForm::eventTreeButtonDown));
	m_treeSolution->addSelectEventHandler(ui::createMethodHandler(this, &SolutionForm::eventTreeSelect));
	m_treeSolution->addEditedEventHandler(ui::createMethodHandler(this, &SolutionForm::eventTreeChange));

	m_menuSolution = gc_new< ui::PopupMenu >();
	m_menuSolution->create();
	m_menuSolution->add(gc_new< ui::MenuItem >(ui::Command(L"Solution.AddProject"), L"Add New Project"));

	m_menuProject = gc_new< ui::PopupMenu >();
	m_menuProject->create();
	m_menuProject->add(gc_new< ui::MenuItem >(ui::Command(L"Project.AddConfiguration"), L"Add New Configuration"));
	m_menuProject->add(gc_new< ui::MenuItem >(ui::Command(L"Project.AddFilter"), L"Add New Filter"));
	m_menuProject->add(gc_new< ui::MenuItem >(ui::Command(L"Project.AddFile"), L"Add New File"));
	m_menuProject->add(gc_new< ui::MenuItem >(ui::Command(L"Project.AddExistingFiles"), L"Add Existing File(s)..."));
	m_menuProject->add(gc_new< ui::MenuItem >(ui::Command(L"Project.Remove"), L"Remove"));

	m_menuConfiguration = gc_new< ui::PopupMenu >();
	m_menuConfiguration->create();
	m_menuConfiguration->add(gc_new< ui::MenuItem >(ui::Command(L"Configuration.Remove"), L"Remove"));

	m_menuFilter = gc_new< ui::PopupMenu >();
	m_menuFilter->create();
	m_menuFilter->add(gc_new< ui::MenuItem >(ui::Command(L"Filter.AddFilter"), L"Add New Filter"));
	m_menuFilter->add(gc_new< ui::MenuItem >(ui::Command(L"Filter.AddFile"), L"Add New File"));
	m_menuFilter->add(gc_new< ui::MenuItem >(ui::Command(L"Filter.AddExistingFiles"), L"Add Existing File(s)..."));
	m_menuFilter->add(gc_new< ui::MenuItem >(ui::Command(L"Filter.Remove"), L"Remove"));

	m_menuFile = gc_new< ui::PopupMenu >();
	m_menuFile->create();
	m_menuFile->add(gc_new< ui::MenuItem >(ui::Command(L"File.Remove"), L"Remove"));

	Ref< ui::Container > pageContainer = gc_new< ui::Container >();
	pageContainer->create(splitter, ui::WsNone, gc_new< ui::FloodLayout >());

	m_pageSolution = gc_new< SolutionPropertyPage >();
	m_pageSolution->create(pageContainer);
	m_pageSolution->hide();

	m_pageProject = gc_new< ProjectPropertyPage >();
	m_pageProject->create(pageContainer);
	m_pageProject->hide();

	m_pageConfiguration = gc_new< ConfigurationPropertyPage >();
	m_pageConfiguration->create(pageContainer);
	m_pageConfiguration->hide();

	// Load MRU registry.
	Ref< Stream > file = FileSystem::getInstance().open(L"SolutionBuilder.mru", traktor::File::FmRead);
	if (file)
	{
		m_mru = xml::XmlDeserializer(file).readObject< MRU >();
		file->close();
	}
	if (!m_mru)
		m_mru = gc_new< MRU >();

	bool loaded = false;
	if (cmdLine.getCount() > 0)
		loaded = loadSolution(cmdLine.getString(0));
	if (!loaded)
	{
		commandNew();
		updateMRU();
	}

	update();
	show();

	startTimer(500);

	return true;
}

void SolutionForm::destroy()
{
	m_menuFile->destroy();
	m_menuFilter->destroy();
	m_menuConfiguration->destroy();
	m_menuProject->destroy();
	m_menuSolution->destroy();
	m_menuBar->destroy();

	ui::Form::destroy();
}

void SolutionForm::updateTitle()
{
	std::wstringstream ss;
	
	ss << TITLE;

	if (m_solution)
	{
		ss << L" - " << (m_solutionFileName.empty() ? L"[Unnamed]" : m_solutionFileName);
		if (isModified())
			ss << L"*";
	}

	setText(ss.str());
}

void SolutionForm::updateSolutionTree()
{
	m_pageSolution->hide();
	m_pageProject->hide();
	m_pageConfiguration->hide();

	Ref< ui::TreeViewState > treeState = m_treeSolution->captureState();

	m_treeSolution->removeAllItems();

	Ref< ui::TreeViewItem > treeSolution = m_treeSolution->createItem(0, m_solution->getName(), 0);
	treeSolution->setData(L"PRIMARY", m_solution);
	treeSolution->setData(L"SOLUTION", m_solution);
	
	RefList< Project >& projects = m_solution->getProjects();
	projects.sort(ProjectSortPredicate());

	for (RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
		createTreeProjectItem(treeSolution, *i);

	m_treeSolution->applyState(treeState);
}

void SolutionForm::updateMRU()
{
	m_menuItemMRU->removeAll();

	std::vector< Path > usedFiles;
	if (!m_mru->getUsedFiles(usedFiles))
		return;

	for (std::vector< Path >::iterator i = usedFiles.begin(); i != usedFiles.end(); ++i)
	{
		Ref< ui::MenuItem > menuItem = gc_new< ui::MenuItem >(ui::Command(L"File.MRU"), i->getPathName());
		menuItem->setData(L"PATH", gc_new< Path >(*i));
		m_menuItemMRU->add(menuItem);
	}
}

bool SolutionForm::isModified() const
{
	return m_solution && DeepHash(m_solution).get() != m_solutionHash;
}

ui::TreeViewItem* SolutionForm::createTreeProjectItem(ui::TreeViewItem* parentItem, Project* project)
{
	Ref< ui::TreeViewItem > treeProject = m_treeSolution->createItem(parentItem, project->getName(), 1);
	treeProject->setData(L"PRIMARY", project);
	treeProject->setData(L"PROJECT", project);

	Ref< ui::TreeViewItem > treeConfigurations = m_treeSolution->createItem(treeProject, L"Configurations", 2, 3);

	RefList< Configuration >& configurations = project->getConfigurations();
	for (RefList< Configuration >::iterator j = configurations.begin(); j != configurations.end(); ++j)
		createTreeConfigurationItem(treeConfigurations, project, *j);

	const RefList< ProjectItem >& items = project->getItems();
	for (RefList< ProjectItem >::const_iterator j = items.begin(); j != items.end(); ++j)
	{
		if (is_a< Filter >(*j))
			createTreeFilterItem(treeProject, project, static_cast< Filter* >(*j));
	}
	for (RefList< ProjectItem >::const_iterator j = items.begin(); j != items.end(); ++j)
	{
		if (is_a< ::File >(*j))
			createTreeFileItem(treeProject, project, static_cast< ::File* >(*j));
	}

	return treeProject;
}

ui::TreeViewItem* SolutionForm::createTreeConfigurationItem(ui::TreeViewItem* parentItem, Project* project, Configuration* configuration)
{
	Ref< ui::TreeViewItem > treeConfiguration = m_treeSolution->createItem(parentItem, configuration->getName(), 5);
	treeConfiguration->setData(L"PRIMARY", configuration);
	treeConfiguration->setData(L"PROJECT", project);
	treeConfiguration->setData(L"CONFIGURATION", configuration);
	return treeConfiguration;
}

ui::TreeViewItem* SolutionForm::createTreeFilterItem(ui::TreeViewItem* parentItem, Project* project, Filter* filter)
{
	Ref< ui::TreeViewItem > treeFilter = m_treeSolution->createItem(parentItem, filter->getName(), 2, 3);
	treeFilter->setData(L"PRIMARY", filter);
	treeFilter->setData(L"PROJECT", project);
	treeFilter->setData(L"FILTER", filter);

	const RefList< ProjectItem >& items = filter->getItems();
	for (RefList< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		if (is_a< Filter >(*i))
			createTreeFilterItem(treeFilter, project, static_cast< Filter* >(*i));
	}
	for (RefList< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		if (is_a< ::File >(*i))
			createTreeFileItem(treeFilter, project, static_cast< ::File* >(*i));
	}

	return treeFilter;
}

ui::TreeViewItem* SolutionForm::createTreeFileItem(ui::TreeViewItem* parentItem, Project* project, ::File* file)
{
	Ref< ui::TreeViewItem > treeFile = m_treeSolution->createItem(parentItem, file->getFileName(), 4);
	treeFile->setData(L"PRIMARY", file);
	treeFile->setData(L"PROJECT", project);
	treeFile->setData(L"FILE", file);
	return treeFile;
}

bool SolutionForm::loadSolution(const traktor::Path& fileName)
{
	Ref< Stream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return false;

	m_solution = xml::XmlDeserializer(file).readObject< Solution >();
	file->close();

	updateSolutionTree();

	m_solutionHash = DeepHash(m_solution).get();
	m_solutionFileName = fileName.getPathName();

	m_mru->usedFile(fileName);

	updateMRU();
	return true;
}

void SolutionForm::commandNew()
{
	if (isModified())
	{
		if (ui::MessageBox::show(this, L"Solution not saved, discard changes?", L"Solution not saved", ui::MbYesNo) == ui::DrNo)
			return;
	}

	m_solution = gc_new< Solution >();
	m_solution->setName(L"Unnamed");
	
	m_solutionHash = DeepHash(m_solution).get();
	m_solutionFileName = L"";

	updateSolutionTree();
	updateTitle();
}

void SolutionForm::commandOpen()
{
	if (isModified())
	{
		if (ui::MessageBox::show(this, L"Solution not saved, discard changes?", L"Solution not saved", ui::MbYesNo) == ui::DrNo)
			return;
	}

	ui::FileDialog fileDialog;
	fileDialog.create(this, L"Open solution", L"SolutionBuilder solutions;*.xms;All files;*.*");
	
	Path filePath;
	if (fileDialog.showModal(filePath))
	{
		if (!loadSolution(filePath))
			ui::MessageBox::show(this, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	fileDialog.destroy();

	updateTitle();
}

void SolutionForm::commandSave(bool saveAs)
{
	bool cancelled = false;
	bool result = false;
	Path filePath;

	if (saveAs || m_solutionFileName.empty())
	{
		ui::FileDialog fileDialog;
		fileDialog.create(this, L"Save solution as", L"SolutionBuilder solutions;*.xms", true);
		cancelled = !(fileDialog.showModal(filePath) == ui::DrOk);
		fileDialog.destroy();
	}
	else
		filePath = m_solutionFileName;

	if (cancelled)
		return;

	Ref< Stream > file = FileSystem::getInstance().open(filePath, traktor::File::FmWrite);
	if (file)
	{
		result = xml::XmlSerializer(file).writeObject(m_solution);
		file->close();
	}

	if (result)
	{
		m_solutionHash = DeepHash(m_solution).get();
		m_solutionFileName = filePath.getPathName();

		m_mru->usedFile(filePath);

		updateMRU();
	}
	else
		ui::MessageBox::show(this, L"Unable to save solution", L"Error", ui::MbIconExclamation | ui::MbOk);

	updateTitle();
}

bool SolutionForm::commandExit()
{
	if (isModified())
	{
		if (ui::MessageBox::show(this, L"Solution not saved, discard changes?", L"Solution not saved", ui::MbYesNo) == ui::DrNo)
			return false;
	}

	// Save MRU registry.
	Ref< Stream > file = FileSystem::getInstance().open(L"SolutionBuilder.mru", traktor::File::FmWrite);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(m_mru);
		file->close();
	}

	ui::Application::getInstance().exit(0);
	return true;
}

void SolutionForm::eventTimer(ui::Event*)
{
	updateTitle();
}

void SolutionForm::eventClose(ui::Event* event)
{
	if (!commandExit())
	{
		checked_type_cast< ui::CloseEvent* >(event)->cancel();
		checked_type_cast< ui::CloseEvent* >(event)->consume();
	}
}

void SolutionForm::eventShortcut(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	if (command == L"File.New")
		commandNew();
	else if (command == L"File.Open")
		commandOpen();
	else if (command == L"File.Save")
		commandSave(false);
	else if (command == L"File.SaveAs")
		commandSave(true);
	else if (command == L"File.Exit")
		commandExit();
}

void SolutionForm::eventMenuClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	if (command == L"File.New")
		commandNew();
	else if (command == L"File.Open")
		commandOpen();
	else if (command == L"File.Save")
		commandSave(false);
	else if (command == L"File.SaveAs")
		commandSave(true);
	else if (command == L"File.MRU")
	{
		Ref< Path > path = checked_type_cast< ui::MenuItem* >(event->getItem())->getData< Path >(L"PATH");
		T_ASSERT (path);

		Ref< Stream > file = FileSystem::getInstance().open(*path, traktor::File::FmRead);
		if (file)
		{
			m_solution = xml::XmlDeserializer(file).readObject< Solution >();
			file->close();

			updateSolutionTree();

			m_solutionHash = DeepHash(m_solution).get();
			m_solutionFileName = path->getPathName();

			m_mru->usedFile(*path);

			updateMRU();
		}
		else
			ui::MessageBox::show(this, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}
	else if (command == L"File.Exit")
		commandExit();
	else if (command == L"Tools.AddMultipleConfigurations")
	{
		AddMultipleConfigurations addMultipleConfigurations;
		addMultipleConfigurations.execute(this, m_solution);
		updateSolutionTree();
	}
	else if (command == L"Tools.EditConfigurations")
	{
		EditConfigurations editConfigurations;
		editConfigurations.execute(this, m_solution);
		updateSolutionTree();
	}
	else if (command == L"Tools.ImportProject")
	{
		ImportProject importProject;
		importProject.execute(this, m_solution);
		updateSolutionTree();
	}
	else if (command == L"Tools.ImportMsvcProject")
	{
		ImportMsvcProject importMsvcProject;
		importMsvcProject.execute(this, m_solution, m_solutionFileName);
		updateSolutionTree();
	}
}

void SolutionForm::eventTreeButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);

	if (mouseEvent->getButton() != ui::MouseEvent::BtRight)
		return;

	Ref< ui::TreeViewItem > selectedItem = m_treeSolution->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< Solution > solution = selectedItem->getData< Solution >(L"PRIMARY");
	if (solution)
	{
		if (m_menuSolution->show(m_treeSolution, mouseEvent->getPosition()))
		{
			Ref< Project > project = gc_new< Project >();
			project->setName(L"Unnamed");

			solution->addProject(project);

			createTreeProjectItem(selectedItem, project);
			selectedItem->expand();
		}
	}

	Ref< Project > project = selectedItem->getData< Project >(L"PRIMARY");
	if (project)
	{
		Ref< ui::MenuItem > menuItem = m_menuProject->show(m_treeSolution, mouseEvent->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"Project.AddConfiguration")
			{
				Ref< Configuration > configuration = gc_new< Configuration >();
				configuration->setName(L"Unnamed");

				project->addConfiguration(configuration);

				createTreeConfigurationItem(selectedItem->findChild(L"Configurations"), project, configuration);
				selectedItem->findChild(L"Configurations")->expand();
			}
			else if (command == L"Project.AddFilter")
			{
				Ref< Filter > filter = gc_new< Filter >();
				filter->setName(L"Unnamed");

				project->addItem(filter);

				createTreeFilterItem(selectedItem, project, filter);
				selectedItem->expand();
			}
			else if (command == L"Project.AddFile")
			{
				Ref< ::File > file = gc_new< ::File >();
				file->setFileName(L"*.*");

				project->addItem(file);

				createTreeFileItem(selectedItem, project, file);
				selectedItem->expand();
			}
			else if (command == L"Project.AddExistingFiles")
			{
				ui::FileDialog fileDialog;
				if (fileDialog.create(this, L"Select file(s)...", L"All files;*.*"))
				{
					std::vector< traktor::Path > paths;
					if (fileDialog.showModal(paths) == ui::DrOk)
					{
						traktor::Path sourcePath = FileSystem::getInstance().getAbsolutePath(project->getSourcePath());

						for (std::vector< traktor::Path >::iterator i = paths.begin(); i != paths.end(); ++i)
						{
							traktor::Path relativePath;
							if (FileSystem::getInstance().getRelativePath(*i, sourcePath, relativePath))
							{
								Ref< ::File > file = gc_new< ::File >();
								file->setFileName(relativePath.getPathName());

								project->addItem(file);

								createTreeFileItem(selectedItem, project, file);
								selectedItem->expand();
							}
							else
								traktor::log::error << L"Unable to get relative path from \"" << i->getPathName() << L"\", not accessible from project's source path?" << Endl;
						}
					}
					fileDialog.destroy();
				}
			}
			else if (command == L"Project.Remove")
			{
				m_solution->removeProject(project);

				m_treeSolution->removeItem(selectedItem);
			}
		}
	}

	Ref< Configuration > configuration = selectedItem->getData< Configuration >(L"PRIMARY");
	if (configuration)
	{
		Ref< ui::MenuItem > menuItem = m_menuConfiguration->show(m_treeSolution, mouseEvent->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"Configuration.Remove")
			{
				Ref< Project > project = selectedItem->getData< Project >(L"PROJECT");
				T_ASSERT (project);

				project->removeConfiguration(configuration);

				m_treeSolution->removeItem(selectedItem);
			}
		}
	}

	Ref< Filter > filter = selectedItem->getData< Filter >(L"PRIMARY");
	if (filter)
	{
		Ref< Project > project = selectedItem->getData< Project >(L"PROJECT");
		T_ASSERT (project);

		Ref< ui::MenuItem > menuItem = m_menuFilter->show(m_treeSolution, mouseEvent->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"Filter.AddFilter")
			{
				Ref< Filter > childFilter = gc_new< Filter >();
				childFilter->setName(L"Unnamed");

				filter->addItem(childFilter);

				createTreeFilterItem(selectedItem, project, childFilter);
				selectedItem->expand();
			}
			else if (command == L"Filter.AddFile")
			{
				Ref< ::File > file = gc_new< ::File >();
				file->setFileName(filter->getName() + L"/*.*");

				filter->addItem(file);

				createTreeFileItem(selectedItem, project, file);
				selectedItem->expand();
			}
			else if (command == L"Filter.AddExistingFiles")
			{
				ui::FileDialog fileDialog;
				if (fileDialog.create(this, L"Select file(s)...", L"All files;*.*"))
				{
					std::vector< traktor::Path > paths;
					if (fileDialog.showModal(paths) == ui::DrOk)
					{
						traktor::Path sourcePath = FileSystem::getInstance().getAbsolutePath(project->getSourcePath());

						for (std::vector< traktor::Path >::iterator i = paths.begin(); i != paths.end(); ++i)
						{
							traktor::Path relativePath;
							if (FileSystem::getInstance().getRelativePath(*i, sourcePath, relativePath))
							{
								Ref< ::File > file = gc_new< ::File >();
								file->setFileName(relativePath.getPathName());

								filter->addItem(file);

								createTreeFileItem(selectedItem, project, file);
								selectedItem->expand();
							}
							else
								traktor::log::error << L"Unable to get relative path from \"" << i->getPathName() << L"\", not accessible from project's source path?" << Endl;
						}
					}
					fileDialog.destroy();
				}
			}
			else if (command == L"Filter.Remove")
			{
				Ref< ui::TreeViewItem > parentItem = selectedItem->getParent();
				if (parentItem)
				{
					Ref< Filter > parentFilter = parentItem->getData< Filter >(L"PRIMARY");
					if (parentFilter)
						parentFilter->removeItem(filter);

					Ref< Project > parentProject = parentItem->getData< Project >(L"PRIMARY");
					if (parentProject)
						parentProject->removeItem(filter);

					m_treeSolution->removeItem(selectedItem);
				}
			}
		}
	}

	Ref< ::File > file = selectedItem->getData< ::File >(L"PRIMARY");
	if (file)
	{
		Ref< ui::MenuItem > menuItem = m_menuFile->show(m_treeSolution, mouseEvent->getPosition());
		if (menuItem)
		{
			const ui::Command& command = menuItem->getCommand();
			if (command == L"File.Remove")
			{
				Ref< ui::TreeViewItem > parentItem = selectedItem->getParent();
				if (parentItem)
				{
					Ref< Filter > parentFilter = parentItem->getData< Filter >(L"PRIMARY");
					if (parentFilter)
						parentFilter->removeItem(file);

					Ref< Project > parentProject = parentItem->getData< Project >(L"PRIMARY");
					if (parentProject)
						parentProject->removeItem(file);

					m_treeSolution->removeItem(selectedItem);
				}
			}
		}
	}

	m_treeSolution->update();
}

void SolutionForm::eventTreeSelect(ui::Event* event)
{
	Ref< ui::TreeViewItem > treeItem = checked_type_cast< ui::TreeViewItem* >(
		checked_type_cast< ui::CommandEvent* >(event)->getItem()
	);

	m_treeSolution->setFocus();

	m_pageSolution->hide();
	m_pageProject->hide();
	m_pageConfiguration->hide();

	if (!treeItem)
		return;

	Ref< Solution > solution = treeItem->getData< Solution >(L"PRIMARY");
	if (solution)
	{
		m_pageSolution->show();
		m_pageSolution->set(m_solution);
	}

	Ref< Project > project = treeItem->getData< Project >(L"PRIMARY");
	if (project)
	{
		m_pageProject->show();
		m_pageProject->set(m_solution, project);
	}

	Ref< Configuration > configuration = treeItem->getData< Configuration >(L"CONFIGURATION");
	if (configuration)
	{
		m_pageConfiguration->show();
		m_pageConfiguration->set(configuration);
	}

	update();
}

void SolutionForm::eventTreeChange(ui::Event* event)
{
	Ref< ui::TreeViewItem > treeItem = static_cast< ui::TreeViewItem* >(
		static_cast< ui::CommandEvent* >(event)->getItem()
	);

	Ref< Solution > solution = treeItem->getData< Solution >(L"PRIMARY");
	if (solution)
		solution->setName(treeItem->getText());

	Ref< Project > project = treeItem->getData< Project >(L"PRIMARY");
	if (project)
		project->setName(treeItem->getText());

	Ref< Configuration > configuration = treeItem->getData< Configuration >(L"PRIMARY");
	if (configuration)
		configuration->setName(treeItem->getText());

	Ref< Filter > filter = treeItem->getData< Filter >(L"PRIMARY");
	if (filter)
		filter->setName(treeItem->getText());

	Ref< ::File > file = treeItem->getData< ::File >(L"PRIMARY");
	if (file)
		file->setFileName(treeItem->getText());

	event->consume();
}
