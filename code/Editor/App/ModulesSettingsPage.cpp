#include "Editor/App/ModulesSettingsPage.h"
#include "Editor/Settings.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/Button.h"
#include "Ui/ListBox.h"
#include "Ui/Custom/InputDialog.h"
#include "I18N/Text.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.ModulesSettingsPage", ModulesSettingsPage, ISettingsPage)

bool ModulesSettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = gc_new< ui::Container >();
	if (!container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"100%,*", 0, 4)))
		return false;

	m_listModules = gc_new< ui::ListBox >();
	m_listModules->create(container, i18n::Text(L"EDITOR_SETTINGS_DEPENDENT_MODULES"), ui::ListBox::WsDefault | ui::ListBox::WsSort);

	Ref< ui::Container > containerModulesTools = gc_new< ui::Container >();
	containerModulesTools->create(container, ui::WsNone, gc_new< ui::TableLayout >(L"*,*", L"100%", 0, 4));

	Ref< ui::Button > buttonAddModule = gc_new< ui::Button >();
	buttonAddModule->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_ADD_MODULE"));
	buttonAddModule->addClickEventHandler(ui::createMethodHandler(this, &ModulesSettingsPage::eventButtonAddModuleClick));

	Ref< ui::Button > buttonRemoveModule = gc_new< ui::Button >();
	buttonRemoveModule->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_REMOVE_MODULE"));
	buttonRemoveModule->addClickEventHandler(ui::createMethodHandler(this, &ModulesSettingsPage::eventButtonRemoveModuleClick));

	const std::vector< std::wstring >& modules = settings->getProperty< PropertyStringArray >(L"Editor.Modules");
	for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
		m_listModules->add(*i);

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_MODULES"));
	return true;
}

void ModulesSettingsPage::destroy()
{
}

bool ModulesSettingsPage::apply(Settings* settings)
{
	std::vector< std::wstring > modules;
	for (int32_t i = 0; i < m_listModules->count(); ++i)
		modules.push_back(m_listModules->getItem(i));
	settings->setProperty< PropertyStringArray >(L"Editor.Modules", modules);
	return true;
}

void ModulesSettingsPage::eventButtonAddModuleClick(ui::Event* event)
{
	ui::custom::InputDialog::Field fields[] =
	{
		{ i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_NAME"), L"", 0 }
	};

	ui::custom::InputDialog inputDialog;
	inputDialog.create(
		m_listModules,
		i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_TITLE"),
		i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_MESSAGE"),
		fields,
		sizeof_array(fields)
	);
	if (inputDialog.showModal() == ui::DrOk && !fields[0].value.empty())
		m_listModules->add(fields[0].value);
	inputDialog.destroy();
}

void ModulesSettingsPage::eventButtonRemoveModuleClick(ui::Event* event)
{
	int selectedItem = m_listModules->getSelected();
	if (selectedItem >= 0)
		m_listModules->remove(selectedItem);
}

	}
}
