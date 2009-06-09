#if defined(_WIN32)
#include <windows.h>
#endif
#include <Ui/Application.h>
#if defined(_WIN32)
#include <Ui/Win32/EventLoopWin32.h>
#include <Ui/Win32/WidgetFactoryWin32.h>
#else
#include <Ui/Wx/EventLoopWx.h>
#include <Ui/Wx/WidgetFactoryWx.h>
#endif
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/Split.h>
#include "DroneForm.h"

using namespace traktor;

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
{
	std::vector< std::wstring > argv;

	TCHAR szFilename[MAX_PATH] = _T("");
	GetModuleFileName(NULL, szFilename, sizeof(szFilename));
	argv.push_back(tstows(szFilename));

	Split< std::wstring >::any(mbstows(szCmdLine), L" \t", argv);
	CommandLine cmdLine(argv);
#endif

#if defined(_WIN32)
	ui::Application::getInstance().initialize(
		new ui::EventLoopWin32(),
		new ui::WidgetFactoryWin32()
	);
#else
	ui::Application::getInstance().initialize(
		new ui::EventLoopWx(),
		new ui::WidgetFactoryWx()
	);
#endif

	drone::DroneForm form;
	if (form.create(cmdLine))
	{
		ui::Application::getInstance().execute();
		form.destroy();
	}

	ui::Application::getInstance().finalize();
	return 0;
}
