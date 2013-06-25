@echo off

set TRAKTOR_HOME=%~dp0
set STEAMWORKS_SDK=%TRAKTOR_HOME%3rdp\steamworks-sdk-123a
set FBX_SDK=%TRAKTOR_HOME%3rdp\FbxSdk\2013.1
set BULLET_SDK=%TRAKTOR_HOME%3rdp\bullet-2.80-rev2531

set CLANG_BIN=%TRAKTOR_HOME%3rdp\clang
set EMSCRIPTEN=%TRAKTOR_HOME%3rdp\emscripten
set EMSCRIPTEN_ROOT=%EMSCRIPTEN%
set EMCC_BIN=%EMSCRIPTEN%
set EMCC_WEBBROWSER_EXE="c:\Program Files (x86)\Google\Chrome\Application\chrome.exe"
set LLVM_ROOT=%CLANG_BIN%
set TEMP_DIR=%TRAKTOR_HOME%build\emscripten\tmp

set PATH=%PATH%;%TRAKTOR_HOME%build\win32\releaseshared
set PATH=%PATH%;%TRAKTOR_HOME%build\win32\debugshared

set PATH=%PATH%;%STEAMWORKS_SDK%\redistributable_bin
set PATH=%PATH%;%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x86
set PATH=%PATH%;%TRAKTOR_HOME%3rdp\POWERVR SDK\OGLES2_WINDOWS_PCEMULATION_2.07.27.0484\Builds\OGLES2\WindowsPC\Lib
set PATH=%PATH%;%TRAKTOR_HOME%3rdp\POWERVR SDK\OGLES2_WINDOWS_PCEMULATION_2.07.27.0484\Utilities\PVRTC\dll
set PATH=%PATH%;%SCE_PS3_ROOT%\host-win32\Cg\bin
set PATH=%PATH%;%FBX_SDK%\lib\vs2008\x86
