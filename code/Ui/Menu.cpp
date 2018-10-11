/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MenuShell.h"
#include "Ui/ToolForm.h"
#include "Ui/Itf/IWidgetFactory.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Menu", Menu, Object)

void Menu::add(MenuItem* item)
{
	m_items.push_back(item);
}

const RefArray< MenuItem >& Menu::getItems() const
{
	return m_items;
}

Ref< Widget > Menu::show(Widget* parent, const Point& at) const
{
	if (!parent)
		return nullptr;

	Ref< ToolForm > form = new ToolForm();
	if (!form->create(parent, L"", 0, 0, WsTop, new FloodLayout()))
		return nullptr;

	Ref< MenuShell > shell = new MenuShell();
	if (!shell->create(form))
		return nullptr;

	shell->addEventHandler< MenuClickEvent >([=](MenuClickEvent* e) {
		MenuClickEvent clickEvent(form, e->getItem(), e->getCommand());
		form->raiseEvent(&clickEvent);
	});
	shell->addEventHandler< FocusEvent >([=](FocusEvent* e) {
		if (!form->containFocus())
		{
			MenuClickEvent clickEvent(form, nullptr, Command());
			form->raiseEvent(&clickEvent);
		}
	});

	for (auto item : m_items)
		shell->add(item);

	// Resize form to shell size.
	form->fit();

	// Place form at given position.
	auto rc = form->getRect();
	form->setRect(Rect(
		parent->clientToScreen(at),
		rc.getSize()
	));

	// Show form.
	form->show();

	// Set focus to shell, if it looses focus then we close menu.
	shell->setFocus();

	return form;
}

const MenuItem* Menu::showModal(Widget* parent, const Point& at, int32_t width, int32_t maxItems) const
{
	const MenuItem* selectedItem = nullptr;
	bool going = true;

	if (!parent || m_items.empty())
		return nullptr;

	Ref< ToolForm > form = new ToolForm();
	if (!form->create(parent, L"", 0, 0, WsTop, new FloodLayout()))
		return nullptr;

	Ref< MenuShell > shell = new MenuShell();
	if (!shell->create(form, maxItems))
		return nullptr;

	shell->addEventHandler< MenuClickEvent >([&](MenuClickEvent* e) {
		selectedItem = e->getItem();
		going = false;
	});
	shell->addEventHandler< FocusEvent >([&](FocusEvent* e) {
		if (!form->containFocus())
		{
			selectedItem = nullptr;
			going = false;
		}
	});

	for (auto item : m_items)
		shell->add(item);

	// Resize form to shell size.
	form->fit();

	// Place form at given position.
	auto rc = form->getRect();
	auto sz = rc.getSize();
	
	if (width >= 0)
		sz.cx = width;

	Rect rcForm(
		parent->clientToScreen(at),
		sz
	);

	// Ensure form is placed inside desktop.
	std::list< Rect > desktopRects;
	Application::getInstance()->getWidgetFactory()->getDesktopRects(desktopRects);
	for (auto r : desktopRects)
	{
		if (rcForm.left < r.left)
			rcForm = rcForm.offset(-(rcForm.left - r.left), 0);
		if (rcForm.right > r.right)
			rcForm = rcForm.offset(-(rcForm.right - r.right), 0);
		if (rcForm.top < r.top)
			rcForm = rcForm.offset(0, -(rcForm.top - r.top));
		if (rcForm.bottom > r.bottom)
			rcForm = rcForm.offset(0, -(rcForm.bottom - r.bottom));
	}

	form->setRect(rcForm);

	// Show form.
	form->show();

	// Set focus to shell, if it looses focus then we close menu.
	shell->setFocus();

	while (going)
	{
		if (!Application::getInstance()->process())
			break;
	}

	safeDestroy(form);
	return selectedItem;
}

	}
}
