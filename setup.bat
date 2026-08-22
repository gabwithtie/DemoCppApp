@echo off
SETLOCAL EnableDelayedExpansion
cls

echo ===================================================
echo    Clang, CMake, and Ninja Automated Setup
echo ===================================================
echo.

:: Check for Administrative Privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] Please right-click this file and select "Run as administrator".
    echo.
    pause
    exit /b
)

:: FIX 0x8a15005e: Temporarily disable SSL/MSStore certificate pinning
echo Applying WinGet Certificate Error Patch...
winget settings --enable BypassCertificatePinningForMicrosoftStore
echo.

:: 1. Install Tools via Winget forcing the main 'winget' repository source
echo [1/3] Installing LLVM (Clang Compiler)...
winget install LLVM.LLVM --source winget --silent --accept-source-agreements --accept-package-agreements
if %errorLevel% neq 0 echo [WARNING] LLVM install exited with code %errorLevel%
echo.

echo [2/3] Installing CMake...
winget install Kitware.CMake --source winget --silent --accept-source-agreements --accept-package-agreements
if %errorLevel% neq 0 echo [WARNING] CMake install exited with code %errorLevel%
echo.

echo [3/3] Installing Ninja Build System...
winget install Ninja-build.Ninja --source winget --silent --accept-source-agreements --accept-package-agreements
if %errorLevel% neq 0 echo [WARNING] Ninja install exited with code %errorLevel%
echo.

:: Re-enable certificate pinning for system safety
echo Restoring standard certificate configurations...
winget settings --disable BypassCertificatePinningForMicrosoftStore
echo.

:: 2. Dynamically update PATH for the current CMD window session
echo Refreshing PATH environment variables for this window...
for /f "tokens=2*" %%A in ('reg query "HKLM\System\CurrentControlSet\Control\Session Manager\Environment" /v Path') do set "SYS_PATH=%%B"
for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v Path') do set "USER_PATH=%%B"
set "PATH=%SYS_PATH%;%USER_PATH%;C:\Program Files\LLVM\bin"

:: 3. Verification Test
echo.
echo ===================================================
echo                Verification Test
echo ===================================================

where clang++ >nul 2>&1
if %errorLevel% equ 0 (
    for /f "delims=" %%A in ('clang++ --version') do (
        echo [SUCCESS] Clang Installed: %%A
        goto :check_cmake
    )
) else (
    echo [FAIL] Clang compiler was not found in PATH.
)

:check_cmake
where cmake >nul 2>&1
if %errorLevel% equ 0 (
    for /f "delims=" %%A in ('cmake --version') do (
        echo [SUCCESS] CMake Installed: %%A
        goto :check_ninja
    )
) else (
    echo [FAIL] CMake was not found in PATH.
)

:check_ninja
where ninja >nul 2>&1
if %errorLevel% equ 0 (
    for /f "delims=" %%A in ('ninja --version') do (
        echo [SUCCESS] Ninja Installed: %%A
        goto :end
    )
) else (
    echo [FAIL] Ninja was not found in PATH.
)

:end
echo.
echo ===================================================
echo Setup complete. Please RESTART VS Code before usage!
echo ===================================================
echo.
pause
