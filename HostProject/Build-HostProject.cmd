@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%") do set "PROJECT_DIR=%%~fI"
for %%I in ("%PROJECT_DIR%..") do set "REPO_ROOT=%%~fI"

set "ENGINE_ROOT=I:\UE_5.7"
set "BUILD_BAT=%ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "UPROJECT=%PROJECT_DIR%NarrRailHost.uproject"
set "PLUGIN_SOURCE=%REPO_ROOT%\NarrRail"
set "PLUGIN_LINK=%PROJECT_DIR%Plugins\NarrRail"

if not exist "%BUILD_BAT%" (
  echo [ERROR] Unreal Build.bat not found:
  echo         %BUILD_BAT%
  pause
  exit /b 1
)

if not exist "%UPROJECT%" (
  echo [ERROR] Host project not found:
  echo         %UPROJECT%
  pause
  exit /b 1
)

if not exist "%PROJECT_DIR%Plugins" (
  mkdir "%PROJECT_DIR%Plugins" >nul 2>nul
)

if not exist "%PLUGIN_LINK%" (
  if not exist "%PLUGIN_SOURCE%" (
    echo [ERROR] Plugin source not found:
    echo         %PLUGIN_SOURCE%
    pause
    exit /b 1
  )

  mklink /J "%PLUGIN_LINK%" "%PLUGIN_SOURCE%" >nul
  if errorlevel 1 (
    echo [ERROR] Failed to create plugin junction:
    echo         %PLUGIN_LINK% ^> %PLUGIN_SOURCE%
    pause
    exit /b 1
  )
)

echo EngineRoot: %ENGINE_ROOT%
echo Project: %UPROJECT%
echo Target: UnrealEditor Win64 Development

echo.
call "%BUILD_BAT%" UnrealEditor Win64 Development -Project="%UPROJECT%" -WaitMutex -NoHotReloadFromIDE
set "EXIT_CODE=%ERRORLEVEL%"

echo.
if "%EXIT_CODE%"=="0" (
  echo Build succeeded.
) else (
  echo Build failed. Exit code: %EXIT_CODE%
)

if not defined NO_PAUSE pause
exit /b %EXIT_CODE%
