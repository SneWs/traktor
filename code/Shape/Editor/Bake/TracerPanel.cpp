/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/TracerPanel.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/CheckBox.h"
#include "Ui/ProgressBar.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerPanel", TracerPanel, ui::Container)

TracerPanel::TracerPanel(editor::IEditor* editor)
:	m_editor(editor)
,	m_idle(true)
{
}

bool TracerPanel::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,*,*", 8_ut, 4_ut)))
		return false;

	setText(i18n::Text(L"SHAPE_EDITOR_TRACER_PANEL"));

	m_checkEnable = new ui::CheckBox();
	m_checkEnable->create(this, i18n::Text(L"SHAPE_EDITOR_TRACER_ENABLE"), m_editor->getSettings()->getProperty< bool >(L"BakePipelineOperator.Enable", true));
	m_checkEnable->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent* event) {
		auto settings = m_editor->checkoutGlobalSettings();
		settings->setProperty< PropertyBoolean >(L"BakePipelineOperator.Enable", m_checkEnable->isChecked());
		m_editor->commitGlobalSettings();

		m_progressBar->setEnable(m_checkEnable->isChecked());
		m_checkTraceImages->setEnable(m_checkEnable->isChecked());
		update();
	});

	m_checkTraceImages = new ui::CheckBox();
	m_checkTraceImages->create(this, i18n::Text(L"SHAPE_EDITOR_TRACER_IMAGES"), m_editor->getSettings()->getProperty< bool >(L"BakePipelineOperator.TraceImages", false));
	m_checkTraceImages->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent* event) {
		auto settings = m_editor->checkoutGlobalSettings();
		settings->setProperty< PropertyBoolean >(L"BakePipelineOperator.TraceImages", m_checkTraceImages->isChecked());
		m_editor->commitGlobalSettings();
	});

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0_ut, 8_ut));

	m_progressBar = new ui::ProgressBar();
	m_progressBar->create(container, ui::WsDoubleBuffer, 0, 100);
	m_progressBar->setText(i18n::Text(L"SHAPE_EDITOR_TRACER_IDLE"));
	m_progressBar->setEnable(m_checkEnable->isChecked());

	m_buttonAbort = new ui::Button();
	m_buttonAbort->create(container, i18n::Text(L"SHAPE_EDITOR_TRACER_ABORT"));
	m_buttonAbort->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent* event) {
		auto processor = BakePipelineOperator::getTracerProcessor();
		if (processor)
			processor->cancelAll();
	});
	m_buttonAbort->setEnable(false);

	addEventHandler< ui::TimerEvent >([&](ui::TimerEvent* event) {
		auto processor = BakePipelineOperator::getTracerProcessor();
		if (processor)
		{
			auto status = processor->getStatus();
			if (status.active && status.total > 0)
			{
				m_progressBar->setText(status.description);
				m_progressBar->setProgress((status.current * 100) / status.total);
				m_buttonAbort->setEnable(true);
				m_buttonAbort->update();
				m_idle = false;
				update();
			}
			else if (!m_idle)
			{
				m_progressBar->setText(i18n::Text(L"SHAPE_EDITOR_TRACER_IDLE"));
				m_progressBar->setProgress(0);
				m_buttonAbort->setEnable(false);
				m_buttonAbort->update();
				m_idle = true;
				update();
			}
		}
	});

	startTimer(300);
	update();

	return true;
}

	}
}
