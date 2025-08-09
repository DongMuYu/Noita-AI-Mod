@echo off
echo === Building CUDA Sequence Trainer ===

REM Set paths
set PROJECT_ROOT=%~dp0
set BUILD_DIR=%PROJECT_ROOT%build_cuda
set TRAINER_DIR=%PROJECT_ROOT%src\ai\trainer\CUDA_SLTrainer

REM Check CUDA environment
where nvcc >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: CUDA nvcc compiler not found in PATH
    echo Please install CUDA toolkit and add it to PATH
    pause
    exit /b 1
)

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
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%PROJECT_ROOT%bin -DBUILD_CUDA_TRAINER=ON -S "%TRAINER_DIR%" -B .

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
echo Executable location: %PROJECT_ROOT%bin\train_cuda_sequence.exe
echo Data directory: %PROJECT_ROOT%data
echo Models directory: %PROJECT_ROOT%models
echo.
echo Available executables:
echo   - bin\train_cuda_sequence.exe (CUDA-accelerated sequence trainer)
echo.
echo Usage:
echo   - CUDA sequence training: bin\train_cuda_sequence.exe
echo.
echo Prerequisites:
echo   - CUDA Toolkit 11.0 or higher
echo   - cuDNN 8.0 or higher
echo   - NVIDIA GPU with Compute Capability 7.0 or higher
echo.
pause