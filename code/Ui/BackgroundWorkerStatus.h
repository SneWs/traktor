#pragma once

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Ui/BackgroundWorkerDialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! Background worker status report.
 * \ingroup UI
 */
class T_DLLCLASS BackgroundWorkerStatus : public RefCountImpl< BackgroundWorkerDialog::IWorkerStatus >
{
public:
	BackgroundWorkerStatus();

	BackgroundWorkerStatus(int32_t steps);

	void setSteps(int32_t steps);

	void notify(int32_t step, const std::wstring& status);

	virtual bool read(int32_t& outStep, std::wstring& outStatus) override final;

private:
	Semaphore m_lock;
	int32_t m_steps;
	int32_t m_step;
	std::wstring m_status;
};

	}
}

