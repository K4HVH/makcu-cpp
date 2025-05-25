@echo off
echo MAKCU C++ Library - Quick Compilation Test
echo =========================================

REM Check if cl.exe is available
where cl.exe >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Visual Studio compiler not found in PATH.
    echo Please run this from a "Developer Command Prompt" or "x64 Native Tools Command Prompt"
    echo.
    echo Alternatively, open makcu-basic-test.vcxproj in Visual Studio and build there.
    pause
    exit /b 1
)

echo Compiling basic test...
echo.

REM Compile the basic test
cl.exe /EHsc /std:c++17 /I. ^
    basic_test.cpp ^
    makcu-cpp\src\makcu.cpp ^
    makcu-cpp\src\serialport.cpp ^
    setupapi.lib ^
    /Fe:basic_test.exe

if %ERRORLEVEL% neq 0 (
    echo.
    echo *** COMPILATION FAILED ***
    echo Please check the error messages above.
    echo.
    echo Common solutions:
    echo 1. Make sure you're using a Visual Studio Developer Command Prompt
    echo 2. Check that all source files exist
    echo 3. Verify C++17 support is available
    echo.
    pause
    exit /b 1
)

echo.
echo *** COMPILATION SUCCESSFUL ***
echo.
echo Running basic test...
echo ========================

basic_test.exe

if %ERRORLEVEL% neq 0 (
    echo.
    echo Test failed with error code %ERRORLEVEL%
) else (
    echo.
    echo *** ALL TESTS PASSED ***
    echo The library is working correctly!
)

echo.
echo Test completed. Press any key to exit.
pause >nul
