@echo off
echo === Incremental Build aiDev.exe ===

REM Set paths
set BUILD_DIR=build
set BIN_DIR=bin

REM Check if build directory exists
if not exist "%BUILD_DIR%" (
    echo Build directory not found, running full build...
    call build_main.bat
    exit /b
)

REM Check if CMakeLists.txt is newer than build cache
if exist "%BUILD_DIR%\CMakeCache.txt" (
    for %%f in (CMakeLists.txt) do (
        for %%c in ("%BUILD_DIR%\CMakeCache.txt") do (
            if %%~tf gtr %%~tc (
                echo CMakeLists.txt has changed, reconfiguring...
                cd /d "%BUILD_DIR%"
                cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=../%BIN_DIR% ..
                if %errorlevel% neq 0 (
                    echo CMake reconfiguration failed!
                    pause
                    exit /b 1
                )
                cd ..
            )
        )
    )
)

REM Incremental build
echo Running incremental build...
cd /d "%BUILD_DIR%"
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Incremental build failed!
    pause
    exit /b 1
)

cd ..
echo.
echo === Incremental Build Complete ===
echo Executable location: %BIN_DIR%\aiDev.exe
echo.
pause