@echo off
echo === Running Sequence Learning Training ===
echo.
echo Features:
echo - Uses sequence data from sequence_data folder
echo - LSTM-based sequence model training
echo - Temporal consistency learning
echo.

REM Set paths
set PROJECT_ROOT=%~dp0
set BIN_DIR=%PROJECT_ROOT%bin
set DATA_DIR=%PROJECT_ROOT%data
set MODELS_DIR=%PROJECT_ROOT%models

REM Check executable
if not exist "%BIN_DIR%\sequence_trainer.exe" (
    echo Error: sequence_trainer.exe does not exist, please run build_sequence_trainer.bat first
    pause
    exit /b 1
)

REM Create necessary directories
if not exist "%DATA_DIR%\sequence_data" (
    echo Creating sequence data directory...
    mkdir "%DATA_DIR%\sequence_data"
)

if not exist "%MODELS_DIR%" (
    echo Creating models directory...
    mkdir "%MODELS_DIR%"
)

REM Create sequence models subdirectory
if not exist "%MODELS_DIR%\sequence_models" (
    echo Creating sequence models subdirectory...
    mkdir "%MODELS_DIR%\sequence_models"
)

REM Check sequence training data
if exist "%DATA_DIR%\sequence_data\collected_data.bin" (
    echo Found sequence training data: %DATA_DIR%\sequence_data\collected_data.bin
) else (
    echo Warning: Sequence training data file not found
    echo Please run the game with data collection enabled to generate sequence data
    echo Data will be saved to: %DATA_DIR%\sequence_data\collected_data.bin
)

echo.
echo Starting sequence learning training...
echo.

REM Run sequence trainer
cd /d "%PROJECT_ROOT%"
"%BIN_DIR%\sequence_trainer.exe"

echo.
echo === Training Complete ===
echo.
echo Check training results:
echo Data directory: %DATA_DIR%\sequence_data
echo Models directory: %MODELS_DIR%\sequence_models
echo.
echo Model files:
dir "%MODELS_DIR%\sequence_models\*.nn" /b 2>nul || echo No model files found
echo.
echo Note: Sequence models are saved with .nn extension
echo Best model: best_sequence_model.nn
pause