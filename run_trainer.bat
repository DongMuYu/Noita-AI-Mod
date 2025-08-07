@echo off
echo === Running Supervised Learning Training (Optimized) ===
echo.
echo Features:
echo - Uses reduced dataset for faster training
echo - Saves intermediate models every 20 epochs
echo - Early stopping mechanism enabled
echo - Optimized training parameters
echo.

REM Set paths
set PROJECT_ROOT=%~dp0
set BIN_DIR=%PROJECT_ROOT%bin
set DATA_DIR=%PROJECT_ROOT%data
set MODELS_DIR=%PROJECT_ROOT%models

REM Check executable
if not exist "%BIN_DIR%\sl_trainer.exe" (
    echo Error: sl_trainer.exe does not exist, please run build_trainer.bat first
    pause
    exit /b 1
)

REM Create necessary directories
if not exist "%DATA_DIR%" (
    echo Creating data directory...
    mkdir "%DATA_DIR%"
)

if not exist "%MODELS_DIR%" (
    echo Creating models directory...
    mkdir "%MODELS_DIR%"
)

REM Create SL models subdirectory
if not exist "%MODELS_DIR%\SL_models" (
    echo Creating SL models subdirectory...
    mkdir "%MODELS_DIR%\SL_models"
)

REM Check training data
if exist "%DATA_DIR%\training_dataset_reduced.csv" (
    echo Found optimized training data: %DATA_DIR%\training_dataset_reduced.csv
) else if exist "%DATA_DIR%\training_dataset.csv" (
    echo Found training data: %DATA_DIR%\training_dataset.csv
    echo Note: Consider using reduced dataset for faster training
) else (
    echo Warning: Training data file not found
    echo Please copy training data file to: %DATA_DIR%\training_dataset_reduced.csv
    echo Or run data processing script first
)

echo.
echo Starting supervised learning training...
echo.

REM Run trainer
cd /d "%PROJECT_ROOT%"
"%BIN_DIR%\sl_trainer.exe"

echo.
echo === Training Complete ===
echo.
echo Check training results:
echo Data directory: %DATA_DIR%
echo Models directory: %MODELS_DIR%
echo.
echo Model files:
dir "%MODELS_DIR%\SL_models\*.bin" /b 2>nul || echo No model files found
echo.
echo Note: Intermediate models are saved with epoch numbers
echo Final models include timestamp: trained_model_reduced_YYYYMMDD_HHMMSS.bin
pause