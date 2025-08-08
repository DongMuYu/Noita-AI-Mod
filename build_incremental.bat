@echo off
setlocal enabledelayedexpansion
echo === Incremental Build aiDev.exe ===

REM Set paths
set BUILD_DIR=build
set BIN_DIR=bin
set CONFIG=Release

REM Check if build directory exists
if not exist "%BUILD_DIR%" (
    echo Build directory not found, running full build...
    call build_main.bat
    exit /b !errorlevel!
)

REM Check if build directory contains valid CMake project
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo Build cache not found, running full build...
    call build_main.bat
    exit /b !errorlevel!
)

REM Check if CMakeLists.txt is newer than build cache
for %%f in (CMakeLists.txt) do (
    for %%c in ("%BUILD_DIR%\CMakeCache.txt") do (
        if %%~tf gtr %%~tc (
            echo CMakeLists.txt has changed, reconfiguring...
            cd /d "%BUILD_DIR%"
            cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=!CONFIG! -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_!CONFIG!=../%BIN_DIR% ..
            if !errorlevel! neq 0 (
                echo CMake reconfiguration failed!
                pause
                exit /b 1
            )
            cd ..
        )
    )
)

REM Check for clean option
if "%1"=="clean" (
    echo Cleaning build directory...
    cd /d "%BUILD_DIR%"
    cmake --build . --target clean --config !CONFIG!
    if !errorlevel! neq 0 (
        echo Clean failed!
        pause
        exit /b 1
    )
    cd ..
)

REM Incremental build
echo Running incremental build...
cd /d "%BUILD_DIR%"
cmake --build . --config !CONFIG!

if !errorlevel! neq 0 (
    echo Incremental build failed!
    echo Please run build_main.bat for a full rebuild.
    pause
    exit /b 1
)

cd ..
echo.
echo === Incremental Build Complete ===
echo Executable location: %BIN_DIR%\aiDev.exe
echo Build configuration: !CONFIG!
echo.
if exist "%BIN_DIR%\aiDev.exe" (
    echo Build successful! Executable size: %%~z"%BIN_DIR%\aiDev.exe" bytes
) else (
    echo Warning: Executable not found in expected location!
)
echo.
pause