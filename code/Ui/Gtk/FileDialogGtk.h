/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_FileDialogGtk_H
#define traktor_ui_FileDialogGtk_H

#include <gtk/gtk.h>
#include "Ui/Itf/IFileDialog.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class FileDialogGtk : public IFileDialog
{
public:
	FileDialogGtk(EventSubject* owner);

	virtual ~FileDialogGtk();

	virtual bool create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int showModal(Path& outPath) T_OVERRIDE T_FINAL;

	virtual int showModal(std::vector< Path >& outPaths) T_OVERRIDE T_FINAL;

private:
	EventSubject* m_owner;
	GtkWidget* m_fileChooser;
};

	}
}

#endif	// traktor_ui_FileDialogGtk_H
