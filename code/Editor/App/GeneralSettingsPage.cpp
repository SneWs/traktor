#include "Editor/App/GeneralSettingsPage.h"
#include "Editor/Settings.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/CheckBox.h"
#include "Ui/Static.h"
#include "Ui/Edit.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.GeneralSettingsPage", GeneralSettingsPage, ISettingsPage)

bool GeneralSettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = gc_new< ui::Container >();
	if (!container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*", 0, 4)))
		return false;

	Ref< ui::Container > containerInner = gc_new< ui::Container >();
	if (!containerInner->create(container, ui::WsNone, gc_new< ui::TableLayout >(L"*,100%", L"*", 0, 0)))
		return false;

	Ref< ui::Static > staticRenderer = gc_new< ui::Static >();
	staticRenderer->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_RENDERER"));

	m_dropRenderSystem = gc_new< ui::DropDown >();
	m_dropRenderSystem->create(containerInner, L"");

	std::wstring renderSystemType = settings->getProperty< PropertyString >(L"Editor.RenderSystem");

	std::vector< const Type* > renderSystemTypes;
	type_of< render::IRenderSystem >().findAllOf(renderSystemTypes, false);

	for (std::vector< const Type* >::const_iterator i = renderSystemTypes.begin(); i != renderSystemTypes.end(); ++i)
	{
		std::wstring name = (*i)->getName();

		int index = m_dropRenderSystem->add(name);
		if (name == renderSystemType)
			m_dropRenderSystem->select(index);
	}

	Ref< ui::Static > staticDictionary = gc_new< ui::Static >();
	staticDictionary->create(containerInner, i18n::Text(L"EDITOR_SETTINGS_DICTIONARY"));

	m_editDictionary = gc_new< ui::Edit >();
	m_editDictionary->create(containerInner, settings->getProperty< PropertyString >(L"Editor.Dictionary"));

	m_checkBuildWhenModified = gc_new< ui::CheckBox >();
	m_checkBuildWhenModified->create(container, i18n::Text(L"EDITOR_SETTINGS_BUILD_WHEN_MODIFIED"));
	m_checkBuildWhenModified->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenModified"));

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_GENERAL"));
	return true;
}

void GeneralSettingsPage::destroy()
{
}

bool GeneralSettingsPage::apply(Settings* settings)
{
	settings->setProperty< PropertyString >(L"Editor.RenderSystem", m_dropRenderSystem->getSelectedItem());
	settings->setProperty< PropertyString >(L"Editor.Dictionary", m_editDictionary->getText());
	settings->setProperty< PropertyBoolean >(L"Editor.BuildWhenModified", m_checkBuildWhenModified->isChecked());
	return true;
}

	}
}
