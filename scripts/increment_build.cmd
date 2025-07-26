@echo off
setlocal EnableDelayedExpansion

:: Check if argument is provided
if "%~1"=="" (
    echo Usage: update_build_number.cmd path\to\src_folder optional-build-number
    exit /b 1
)

:: Convert forward slashes to backslashes and remove quotes
set "BUILD_DIR=%~1"
set "BUILD_DIR=%BUILD_DIR:/=\%"

:: Ensure paths are properly quoted
set "BUILD_FILE=%BUILD_DIR%\buildnumber.txt"
set "HEADER_FILE=%BUILD_DIR%\buildnumber.h"

if "%~2"=="" (
    set "BUILD_NUMBER=0"
    :: Check if file exists
    if exist "%BUILD_FILE%" (
        set /p BUILD_NUMBER=<"%BUILD_FILE%"
    )
    :: Increment
    set /a BUILD_NUMBER+=1
) else (
    set "BUILD_NUMBER=%~2"
)

:: Write new number
echo %BUILD_NUMBER% > "%BUILD_FILE%"

:: Generate header
(
    echo #pragma once
    echo #define BUILD_NUMBER %BUILD_NUMBER%
    echo:
    echo char ^*__cdecl getBuildNumber^(^)^;
    echo int getBuildNumberAsInt^(^)^;
) > "%HEADER_FILE%"

echo Updated build number to %BUILD_NUMBER%
endlocal