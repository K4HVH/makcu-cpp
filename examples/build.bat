@echo off
setlocal enabledelayedexpansion

REM MAKCU C++ Library Examples Build Script (Windows)
REM This script shows how to build applications using the MAKCU library

echo === MAKCU C++ Library Examples Build ===
echo.

REM Check for CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake is not installed or not in PATH
    echo Download from: https://cmake.org/download/
    exit /b 1
)

REM Check for Visual Studio and set up environment
where cl >nul 2>&1
if errorlevel 1 (
    echo Visual Studio compiler not in PATH, attempting to locate Visual Studio...
    
    REM Try to find vswhere.exe (comes with VS 2017+)
    set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!vswhere!" (
        set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
    )
    
    if exist "!vswhere!" (
        REM Find the latest Visual Studio installation
        for /f "usebackq tokens=*" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            set "vsinstall=%%i"
        )
        
        if defined vsinstall (
            echo Setting up Visual Studio environment...
            call "!vsinstall!\VC\Auxiliary\Build\vcvarsall.bat" x64
            if errorlevel 1 (
                echo [ERROR] Failed to set up Visual Studio environment
                exit /b 1
            )
        ) else (
            echo [ERROR] Visual Studio with C++ tools not found
            echo Please install Visual Studio with C++ development tools
            exit /b 1
        )
    ) else (
        echo [ERROR] Visual Studio 2017 or later required
        exit /b 1
    )
)

REM Build with CMake using Visual Studio
echo Building examples with CMake and Visual Studio...
if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo [ERROR] CMake configuration failed
    cd ..
    exit /b 1
)

cmake --build . --config Release --parallel
if errorlevel 1 (
    echo [ERROR] CMake build failed
    cd ..
    exit /b 1
)

if exist "bin\Release\basic_usage.exe" (
    if exist "bin\Release\demo.exe" (
        echo [SUCCESS] Build successful!
        echo.
        echo To run the examples:
        echo   build\bin\Release\basic_usage.exe    # Simple usage example
        echo   build\bin\Release\demo.exe           # Full demo with all features
        echo   build\bin\Release\c_api_test.exe     # C API test
        echo.
        echo To use this in your own project:
        echo 1. Install the MAKCU library: cd .. ^&^& mkdir build ^&^& cd build ^&^& cmake .. ^&^& cmake --build . --config Release ^&^& cmake --build . --target install
        echo 2. Copy the CMakeLists.txt from this examples directory
        echo 3. Use: find_package(makcu-cpp REQUIRED) and target_link_libraries(your_app PRIVATE makcu::makcu-cpp)
    ) else (
        echo [ERROR] demo.exe not found
        cd ..
        exit /b 1
    )
) else (
    echo [ERROR] basic_usage.exe not found
    cd ..
    exit /b 1
)