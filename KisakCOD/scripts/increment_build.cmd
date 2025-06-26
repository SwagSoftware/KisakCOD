@echo off
setlocal EnableDelayedExpansion

:: Check if argument is provided
if "%~1"=="" (
    echo Usage: update_build_number.cmd path\to\src_folder
    exit /b 1
)

set "BUILD_DIR=%~1"
set "BUILD_FILE=%BUILD_DIR%/buildnumber.txt"
set "HEADER_FILE=%BUILD_DIR%/buildnumber.h"
set "BUILD_NUMBER=0"

:: Check if file exists
if exist %BUILD_FILE% (
    set /p BUILD_NUMBER=<%BUILD_FILE%
)

:: Increment
set /a BUILD_NUMBER+=1

:: Write new number
echo %BUILD_NUMBER% > %BUILD_FILE%

:: Generate header
(
    echo #pragma once
    echo #define BUILD_NUMBER %BUILD_NUMBER%
) > %HEADER_FILE%

echo Updated build number to %BUILD_NUMBER%
endlocal