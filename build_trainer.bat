@echo off
echo === Building Supervised Learning Trainer ===

REM Set paths
set PROJECT_ROOT=%~dp0
set BUILD_DIR=%PROJECT_ROOT%build_training
set TRAINER_DIR=%PROJECT_ROOT%src\ai\trainer\SLTrainer

REM Create build directory
if exist "%BUILD_DIR%" (
    echo Cleaning old build directory...
    rmdir /s /q "%BUILD_DIR%"
)

echo Creating build directory...
mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM Configure CMake
echo Configuring CMake...
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%PROJECT_ROOT%bin -S "%TRAINER_DIR%" -B .

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build project
echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo === Build Complete ===
echo Executable location: %PROJECT_ROOT%bin\sl_trainer.exe
echo Data directory: %PROJECT_ROOT%data
echo Models directory: %PROJECT_ROOT%models
echo.
pause