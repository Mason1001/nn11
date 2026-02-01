@echo off
REM ============================================================
REM AccessibilityGamingAssistant - Local Build Script
REM ============================================================
REM هذا السكربت يبني المشروع محلياً على جهازك
REM ============================================================

setlocal enabledelayedexpansion

echo.
echo ========================================
echo  AccessibilityGamingAssistant Builder
echo ========================================
echo.

REM ============================================================
REM Step 1: Detect Qt6 Installation
REM ============================================================
echo [1/5] Searching for Qt6...

set "QT6_DIR="

REM Check common Qt installation paths
for %%P in (
    "C:\Qt\6.10.2\msvc2022_64"
    "C:\Qt\6.9.0\msvc2022_64"
    "C:\Qt\6.8.0\msvc2022_64"
    "C:\Qt\6.7.0\msvc2022_64"
    "C:\Qt\6.6.2\msvc2022_64"
    "C:\Qt\6.6.0\msvc2022_64"
    "C:\Qt\6.5.0\msvc2019_64"
    "D:\Qt\6.10.2\msvc2022_64"
    "D:\Qt\6.8.0\msvc2022_64"
    "D:\Qt\6.6.2\msvc2022_64"
    "%USERPROFILE%\Qt\6.10.2\msvc2022_64"
    "%USERPROFILE%\Qt\6.8.0\msvc2022_64"
    "%USERPROFILE%\Qt\6.6.2\msvc2022_64"
) do (
    if exist "%%~P\lib\cmake\Qt6\Qt6Config.cmake" (
        set "QT6_DIR=%%~P\lib\cmake\Qt6"
        set "QT6_BIN=%%~P\bin"
        echo    Found Qt6 at: %%~P
        goto :qt_found
    )
)

echo    ERROR: Qt6 not found!
echo.
echo    Please install Qt6 from https://qt.io/download
echo    Or set QT6_DIR manually:
echo       set QT6_DIR=C:\Qt\6.x.x\msvc2022_64\lib\cmake\Qt6
echo.
pause
exit /b 1

:qt_found

REM ============================================================
REM Step 2: Detect OpenCV Installation
REM ============================================================
echo [2/5] Searching for OpenCV...

set "OPENCV_DIR="

REM Check vcpkg locations
for %%P in (
    "C:\vcpkg\installed\x64-windows"
    "D:\vcpkg\installed\x64-windows"
    "%USERPROFILE%\vcpkg\installed\x64-windows"
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg\installed\x64-windows"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\vcpkg\installed\x64-windows"
    "%LOCALAPPDATA%\vcpkg\installed\x64-windows"
) do (
    if exist "%%~P\share\opencv4\OpenCVConfig.cmake" (
        set "OPENCV_DIR=%%~P\share\opencv4"
        set "OPENCV_BIN=%%~P\bin"
        echo    Found OpenCV at: %%~P
        goto :opencv_found
    )
)

REM Check standalone OpenCV
for %%P in (
    "C:\opencv\build\x64\vc16"
    "C:\opencv\build\x64\vc15"
    "D:\opencv\build\x64\vc16"
) do (
    if exist "%%~P\lib\OpenCVConfig.cmake" (
        set "OPENCV_DIR=%%~P\lib"
        set "OPENCV_BIN=%%~P\bin"
        echo    Found OpenCV at: %%~P
        goto :opencv_found
    )
)

echo    ERROR: OpenCV not found!
echo.
echo    Please install OpenCV via vcpkg:
echo       vcpkg install opencv4:x64-windows
echo.
echo    Or download from https://opencv.org/releases/
echo.
pause
exit /b 1

:opencv_found

REM ============================================================
REM Step 3: Setup Visual Studio Environment
REM ============================================================
echo [3/5] Setting up Visual Studio environment...

REM Try to find vswhere
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo    ERROR: Visual Studio not found!
    echo    Please install Visual Studio 2022 with C++ workload
    pause
    exit /b 1
)

REM Get VS installation path
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not defined VS_PATH (
    echo    ERROR: Visual Studio C++ tools not found!
    pause
    exit /b 1
)

echo    Found Visual Studio at: %VS_PATH%

REM Initialize VS environment
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo    ERROR: Failed to initialize VS environment
    pause
    exit /b 1
)

echo    Visual Studio environment initialized.

REM ============================================================
REM Step 4: Configure with CMake
REM ============================================================
echo [4/5] Configuring CMake...

REM Create build directory
if not exist build mkdir build
cd build

REM Run CMake
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DQt6_DIR="%QT6_DIR%" ^
    -DOpenCV_DIR="%OPENCV_DIR%"

if errorlevel 1 (
    echo.
    echo    ERROR: CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo    CMake configuration successful!

REM ============================================================
REM Step 5: Build
REM ============================================================
echo [5/5] Building project...

cmake --build . --config Release --parallel

if errorlevel 1 (
    echo.
    echo    ERROR: Build failed!
    cd ..
    pause
    exit /b 1
)

cd ..

REM ============================================================
REM Step 6: Create deployment folder
REM ============================================================
echo.
echo Creating deployment folder...

if not exist deploy mkdir deploy

REM Copy executable
copy /Y "build\Release\AccessibilityGamingAssistant.exe" "deploy\" >nul

REM Deploy Qt DLLs
echo Deploying Qt DLLs...
"%QT6_BIN%\windeployqt.exe" "deploy\AccessibilityGamingAssistant.exe" --release --no-translations >nul 2>&1

REM Copy OpenCV DLLs
echo Copying OpenCV DLLs...
if defined OPENCV_BIN (
    copy /Y "%OPENCV_BIN%\opencv_core*.dll" "deploy\" >nul 2>&1
    copy /Y "%OPENCV_BIN%\opencv_imgproc*.dll" "deploy\" >nul 2>&1
    copy /Y "%OPENCV_BIN%\opencv_highgui*.dll" "deploy\" >nul 2>&1
    copy /Y "%OPENCV_BIN%\opencv_videoio*.dll" "deploy\" >nul 2>&1
    copy /Y "%OPENCV_BIN%\opencv_imgcodecs*.dll" "deploy\" >nul 2>&1
)

echo.
echo ========================================
echo  BUILD SUCCESSFUL!
echo ========================================
echo.
echo  Output: deploy\AccessibilityGamingAssistant.exe
echo.
echo  To run: cd deploy ^&^& AccessibilityGamingAssistant.exe
echo.
echo ========================================

pause
