@echo off

set TRAKTOR_HOME=%~dp0
set STEAMWORKS_SDK="%TRAKTOR_HOME%3rdp\steamworks-sdk-116"

set PATH=%PATH%;"%TRAKTOR_HOME%build\win32\releaseshared"
set PATH=%PATH%;"%TRAKTOR_HOME%build\win32\debugshared"

set PATH=%PATH%;"%STEAMWORKS_SDK%\redistributable_bin"
set PATH=%PATH%;%TRAKTOR_HOME%3rdp\POWERVR SDK\OGLES2_WINDOWS_PCEMULATION_2.07.27.0484\Builds\OGLES2\WindowsPC\Lib
set PATH=%PATH%;%TRAKTOR_HOME%3rdp\POWERVR SDK\OGLES2_WINDOWS_PCEMULATION_2.07.27.0484\Utilities\PVRTC\dll
set PATH=%PATH%;"%SCE_PS3_ROOT%\host-win32\Cg\bin"
