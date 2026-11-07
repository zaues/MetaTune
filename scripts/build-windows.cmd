@echo off
rem Native Windows build with Visual Studio 2022 Build Tools + CMake + Ninja.
rem Usage: scripts\build-windows.cmd [Release|Debug] [out-folder]
rem   -> build\msvc-<config>\MetaTune_artefacts\<Config>\VST3\<PRODUCT_NAME>.vst3, runs the DSP tests,
rem      and copies the bundle to [out-folder] if given.
setlocal
cd /d "%~dp0.."
set CONFIG=%~1
if "%CONFIG%"=="" set CONFIG=Release
set OUT=%~2

for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VSDIR=%%i
if not defined VSDIR (echo Visual Studio C++ Build Tools not found. & exit /b 1)
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1

set BUILD=build\msvc-%CONFIG%
rem Always configure; -U drops cached values so CMakeLists.txt's product name and update repo win.
cmake -S . -B "%BUILD%" -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG% -UPRODUCT_NAME -UMETATUNE_UPDATE_REPO >nul || exit /b 1
cmake --build "%BUILD%" --target MetaTune_VST3 MetaTuneUpdater || exit /b 1

if not exist build\msvc-tests\build.ninja cmake -S Tests -B build\msvc-tests -G Ninja -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build build\msvc-tests --target dsp_tests || exit /b 1
build\msvc-tests\dsp_tests.exe || (echo DSP tests FAILED & exit /b 1)

rem Newest bundle in the artefacts folder (its name follows PRODUCT_NAME).
set VST3DIR=%BUILD%\MetaTune_artefacts\%CONFIG%\VST3
set BUNDLENAME=
for /f "delims=" %%b in ('dir /b /ad /o-d "%VST3DIR%\*.vst3"') do if not defined BUNDLENAME set "BUNDLENAME=%%b"
set BUNDLE=%VST3DIR%\%BUNDLENAME%
echo Built: %BUNDLE%
if not "%OUT%"=="" (
    if exist "%OUT%\%BUNDLENAME%" rmdir /s /q "%OUT%\%BUNDLENAME%"
    xcopy /e /i /q /y "%BUNDLE%" "%OUT%\%BUNDLENAME%" >nul || exit /b 1
    echo Copied to: %OUT%\%BUNDLENAME%
)
