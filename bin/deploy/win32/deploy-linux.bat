@echo off

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

	if not exist "%DEPLOY_OUTPUT_PATH%\reports" mkdir "%DEPLOY_OUTPUT_PATH%\reports"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Textures.html > "%DEPLOY_OUTPUT_PATH%\reports\Textures.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Sounds.html > "%DEPLOY_OUTPUT_PATH%\reports\Sounds.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Shaders.html > "%DEPLOY_OUTPUT_PATH%\reports\Shaders.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Meshes.html > "%DEPLOY_OUTPUT_PATH%\reports\Meshes.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Index.html > "%DEPLOY_OUTPUT_PATH%\reports\Index.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/PhysMeshes.html > "%DEPLOY_OUTPUT_PATH%\reports\PhysMeshes.html"

) else if "%1"=="deploy" (

	pushd %TRAKTOR_HOME%\bin\win32
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% launch-linux.sh > "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd

	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\linux\releaseshared
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% * > "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd

) else if "%1"=="launch" (

	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% Application.config > "%DEPLOY_OUTPUT_PATH%\Launch.log"
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch %DEPLOY_TARGET_HOST% launch-linux.sh %DEPLOY_EXECUTABLE% >> "%DEPLOY_OUTPUT_PATH%\Launch.log"

) else if "%1"=="migrate" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App -s=Migrate -l=Migrate.log

)
