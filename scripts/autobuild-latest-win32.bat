@echo off

:: Setup VC environment variables.
call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"

:: Setup our build environment.
call %~dp0../config.bat

:: Remove old log.
del /F /Q %~dp0autobuild-win32.log

:: Set aggregate output path if not already defined.
if "%AGGREGATE_OUTPUT_PATH%" == "" (
	set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\win32
)

:: Rebuild entire solution.
pushd %TRAKTOR_HOME%

call build-projects-vs2015-win32.bat
call config-vsenv.bat build\win32\version.txt

devenv "build\win32\Traktor Win32.sln" /Build DebugShared /Out %~dp0autobuild-win32.log
devenv "build\win32\Traktor Win32.sln" /Build ReleaseShared /Out %~dp0autobuild-win32.log
devenv "build\win32\Traktor Win32.sln" /Build DebugStatic /Out %~dp0autobuild-win32.log
devenv "build\win32\Traktor Win32.sln" /Build ReleaseStatic /Out %~dp0autobuild-win32.log

popd
