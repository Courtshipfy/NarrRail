@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "PS_SCRIPT=%SCRIPT_DIR%Build-HostProject.ps1"
set "DEFAULT_ENGINE_ROOT=I:\UE_5.7"

if not exist "%PS_SCRIPT%" (
  echo [ERROR] PowerShell build script not found:
  echo         %PS_SCRIPT%
  pause
  exit /b 1
)

set "HAS_ENGINE_ROOT=0"
for %%A in (%*) do (
  if /I "%%~A"=="-EngineRoot" set "HAS_ENGINE_ROOT=1"
)

if "%HAS_ENGINE_ROOT%"=="1" (
  powershell -NoProfile -ExecutionPolicy Bypass -File "%PS_SCRIPT%" %*
) else (
  powershell -NoProfile -ExecutionPolicy Bypass -File "%PS_SCRIPT%" -EngineRoot "%DEFAULT_ENGINE_ROOT%" %*
)

set "EXIT_CODE=%ERRORLEVEL%"

echo.
if "%EXIT_CODE%"=="0" (
  echo Build succeeded.
) else (
  echo Build failed. Exit code: %EXIT_CODE%
)

pause
exit /b %EXIT_CODE%
