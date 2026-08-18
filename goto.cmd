@echo off
setlocal

set "ROOT=%~dp0"
set "GOTO_EXE=%ROOT%bin\goto.exe"
set "TESTS_EXE=%ROOT%bin\goto_tests.exe"

if "%~1"=="" (
    if not exist "%GOTO_EXE%" (
        powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1"
    )
    "%GOTO_EXE%" --repl
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="build" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1"
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="rebuild" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1" -Rebuild
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="test" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1" -RunTests
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="tests" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1" -RunTests
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="demo" (
    if not exist "%GOTO_EXE%" powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1"
    "%GOTO_EXE%" "%ROOT%samples\visual_novel_demo.goto"
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="game" (
    if not exist "%GOTO_EXE%" powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1"
    "%GOTO_EXE%" "%ROOT%samples\game_logic.goto"
    exit /b %ERRORLEVEL%
)

if /i "%~1"=="check" (
    if not exist "%GOTO_EXE%" powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1"
    "%GOTO_EXE%" --check "%~2"
    exit /b %ERRORLEVEL%
)

if not exist "%GOTO_EXE%" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%build.ps1"
)

"%GOTO_EXE%" %*
exit /b %ERRORLEVEL%
