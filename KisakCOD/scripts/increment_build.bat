@echo off
setlocal EnableDelayedExpansion

:: Check if run from KisakCOD root (which must contain src\)
if not exist src\ (
    echo This must be ran from the KisakCOD root which contains CMakeLists.txt
    exit /b 1
)

set "BUILD_FILE=src/buildnumber.txt"
set "HEADER_FILE=src/buildnumber.h"
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