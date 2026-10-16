@echo off
rem ===========================================================================
rem  Nespeculator build script (Windows, MSVC + Ninja).
rem
rem  Requirements
rem    * Visual Studio 2022 or newer with the "Desktop development with C++"
rem      workload (this also provides CMake and Ninja), or an equivalent MSVC
rem      toolchain plus CMake and Ninja on PATH.
rem    * The SDL3 development files, which ship with the repository in
rem      third_party\SDL3 (headers + lib\x64\SDL3.lib + SDL3.dll).
rem
rem  Usage
rem    build.bat              configure (first run) and build into build\
rem    build.bat clean        delete build\ and rebuild from scratch
rem
rem  The script locates vcvars64.bat itself, so it can be double-clicked from
rem  Explorer; inside a Developer Command Prompt the existing environment is
rem  used as-is.
rem ===========================================================================
setlocal enabledelayedexpansion
set "ROOT=%~dp0"

if /i "%~1"=="clean" (
    echo Removing "%ROOT%build" ...
    if exist "%ROOT%build" rmdir /s /q "%ROOT%build"
    if exist "%ROOT%build" (
        echo ERROR: could not remove "%ROOT%build".
        echo        Close any running Nespeculator.exe and try again.
        exit /b 1
    )
)

rem ---------------------------------------------------------- toolchain -----
rem If cl.exe is already on PATH we are inside a Developer Command Prompt and
rem vcvars must not be called a second time.
set "VCVARS="
where cl >nul 2>&1 && set "VCVARS=already"

if not defined VCVARS (
    set "VSPATH="
    rem Preferred: vswhere, which knows about every Visual Studio install.
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * ^
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
            -property installationPath 2^>nul`) do set "VSPATH=%%i"
    )
    rem Fallback: scan the two standard install roots for any year/edition.
    if not defined VSPATH (
        for %%r in ("%ProgramFiles%\Microsoft Visual Studio"
                    "%ProgramFiles(x86)%\Microsoft Visual Studio") do (
            if exist "%%~r" for /d %%y in ("%%~r\*") do (
                if exist "%%~y\VC\Auxiliary\Build\vcvars64.bat" set "VSPATH=%%~y"
            )
        )
    )
    if defined VSPATH set "VCVARS=!VSPATH!\VC\Auxiliary\Build\vcvars64.bat"
)

if not defined VCVARS (
    echo ERROR: no Visual Studio C++ toolchain found.
    echo        Install the "Desktop development with C++" workload, or run
    echo        this script from a Developer Command Prompt.
    exit /b 1
)
if /i not "!VCVARS!"=="already" (
    call "!VCVARS!" >nul 2>&1
    if errorlevel 1 (
        echo VCVARS_FAIL
        exit /b 1
    )
)

rem --------------------------------------------------------------- cmake ----
set "CMAKE=cmake"
where cmake >nul 2>&1
if errorlevel 1 (
    set "CMAKE=!VCINSTALLDIR!Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if not exist "!CMAKE!" (
        echo ERROR: cmake was not found on PATH and is not bundled with Visual Studio.
        exit /b 1
    )
)

rem --------------------------------------------------------------- build ----
rem Configure when there is no cache (a fresh clone, or a build directory that
rem was deleted or never finished configuring).
if not exist "%ROOT%build\CMakeCache.txt" (
    "%CMAKE%" -S "%ROOT%." -B "%ROOT%build" -G Ninja -DCMAKE_BUILD_TYPE=Release
    if errorlevel 1 (
        echo CONFIGURE_FAIL
        exit /b 1
    )
)

"%CMAKE%" --build "%ROOT%build"
if errorlevel 1 (
    echo BUILD_FAIL
    exit /b 1
)

if not exist "%ROOT%build\Nespeculator.exe" (
    echo NO_EXE
    exit /b 1
)

echo BUILD_OK
echo   %ROOT%build\Nespeculator.exe
