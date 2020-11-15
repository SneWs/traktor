#include "I18N/Text.h"
#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/TracerPanel.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/ProgressBar.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerPanel", TracerPanel, ui::Container)

bool TracerPanel::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*", ui::dpi96(16), ui::dpi96(8))))
		return false;

	setText(i18n::Text(L"SHAPE_EDITOR_TRACER_PANEL"));

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, ui::dpi96(16)));

	m_progressBar = new ui::ProgressBar();
	m_progressBar->create(container, ui::WsDoubleBuffer, 0, 100);

	m_buttonAbort = new ui::Button();
	m_buttonAbort->create(container, i18n::Text(L"SHAPE_EDITOR_TRACER_ABORT"));
	m_buttonAbort->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent* event) {
		auto processor = BakePipelineOperator::getTracerProcessor();
		if (processor)
			processor->cancelAll();
	});
	m_buttonAbort->setEnable(false);

	m_staticDescription = new ui::Static();
	m_staticDescription->create(this, i18n::Text(L"SHAPE_EDITOR_TRACER_IDLE"));

	addEventHandler< ui::TimerEvent >([&](ui::TimerEvent* event) {
		auto processor = BakePipelineOperator::getTracerProcessor();
		if (processor)
		{
			auto status = processor->getStatus();
			if (status.active && status.total > 0)
			{
				m_staticDescription->setText(status.description);
				m_progressBar->setProgress((status.current * 100) / status.total);
				m_buttonAbort->setEnable(true);
				m_buttonAbort->update();
			}
			else
			{
				m_staticDescription->setText(i18n::Text(L"SHAPE_EDITOR_TRACER_IDLE"));
				m_progressBar->setProgress(0);
				m_buttonAbort->setEnable(false);
				m_buttonAbort->update();
			}
			update();
		}
	});

	startTimer(200);
	return true;
}

void TracerPanel::destroy()
{
	ui::Container::destroy();
}

	}
}
