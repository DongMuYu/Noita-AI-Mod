@echo off
chcp 65001 >nul
echo Building Sequence Learning Trainer...
echo =====================================

set BUILD_DIR=build_sequence_trainer
set SOURCE_DIR=src\ai\trainer\SLTrainer

REM Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Configure CMake
echo Configuring CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..\%SOURCE_DIR%

if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build project
echo Building project...
cmake --build . --config Release

if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

REM Copy executables to bin directory
echo Copying executables...
if exist Release\*.exe (
    copy Release\*.exe ..\bin\
) else (
    echo Executables not found in Release directory
    echo Checking alternative location...
    if exist *.exe (
        copy *.exe ..\bin\
    ) else (
        echo No executables found to copy
    )
)

REM Return to main directory
cd ..\

echo.
echo Build completed successfully!
echo.
echo Available executables:
echo   - bin\sl_trainer.exe (Traditional supervised learning)
echo   - bin\sequence_trainer.exe (Sequence learning)
echo   - bin\training_manager.exe (Hybrid training manager)
echo.
echo Usage:
echo   - Traditional training: bin\sl_trainer.exe
echo   - Sequence training: bin\sequence_trainer.exe
echo   - Hybrid training: bin\training_manager.exe
echo.
pause