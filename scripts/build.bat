@echo off
REM scripts/build.bat

set BUILD_TYPE=%1
set PRESET=%2

if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release
if "%PRESET%"=="" set PRESET=default

echo Building PixelTree...
echo Build type: %BUILD_TYPE%
echo Preset: %PRESET%

cd /d "%~dp0\.."

REM Configure
cmake --preset %PRESET% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

REM Build
cmake --build build\%PRESET% --config %BUILD_TYPE% -j

REM Test (if enabled)
if "%3"=="test" (
    echo Running tests...
    ctest --test-dir build\%PRESET% --config %BUILD_TYPE% --output-on-failure
)

echo Build complete!