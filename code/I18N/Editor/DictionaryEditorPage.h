#ifndef traktor_i18n_DictionaryEditorPage_H
#define traktor_i18n_DictionaryEditorPage_H

#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace ui
	{
		namespace custom
		{

class GridCellContentChangeEvent;
class GridRowDoubleClickEvent;
class GridView;
class ToolBarButtonClickEvent;

		}
	}

	namespace i18n
	{

class Dictionary;

class T_DLLCLASS DictionaryEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	DictionaryEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::custom::GridView > m_gridDictionary;
	Ref< Dictionary > m_dictionary;
	Ref< Dictionary > m_referenceDictionary;

	void updateGrid();

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventGridRowDoubleClick(ui::custom::GridRowDoubleClickEvent* event);

	void eventGridCellChange(ui::custom::GridCellContentChangeEvent* event);
};

	}
}

#endif	// traktor_i18n_DictionaryEditorPage_H
