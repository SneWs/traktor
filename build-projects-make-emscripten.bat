@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

set MAKE_DIALECT=nmake
set MAKE_TOOLCHAIN=clang
set MAKE_OS=windows
set MAKE_INCLUDE=$(TRAKTOR_HOME)\bin\make-config-emscripten.inc
set MAKE_OBJECT_FILE=%%s.o
set MAKE_STATIC_LIBRARY_FILE=lib%%s.bc
set MAKE_SHARED_LIBRARY_FILE=lib%%s.bc
set MAKE_EXECUTABLE_FILE=%%s.js

%TRAKTOR_HOME%\bin\win32\solutionbuilder ^
	-f=make2 ^
	-make-solution-template=$(TRAKTOR_HOME)\bin\make-solution.sbm ^
	-make-project-template=$(TRAKTOR_HOME)\bin\make-project.sbm ^
	%TRAKTOR_HOME%\TraktorEmscripten.xms
