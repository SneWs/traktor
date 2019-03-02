#pragma once

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class AutoPropertyList;
class ListBox;
class ToolBarButtonClickEvent;

	}

	namespace sound
	{

class SoundAsset;

class SoundBatchDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	SoundBatchDialog(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	bool showModal(RefArray< SoundAsset >& outAssets);

private:
	editor::IEditor* m_editor;
	Ref< ui::ListBox > m_soundList;
	Ref< ui::AutoPropertyList > m_soundPropertyList;

	void addSound();

	void removeSound();

	void eventSoundListToolClick(ui::ToolBarButtonClickEvent* event);

	void eventSoundListSelect(ui::SelectionChangeEvent* event);
};

	}
}

