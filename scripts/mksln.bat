@echo off
setlocal EnableDelayedExpansion

if exist CMakeLists.txt (
    pushd
    mkdir build
    cd build
    cmake -G "Visual Studio 17 2022" -A Win32 ..
    popd
) else (
    echo You must run this from the ROOT directory
    echo Usage: scripts/mksln.bat
    exit /b 1
)

