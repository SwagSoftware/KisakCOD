@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul
cd /d "%~dp0"
echo === FILE ===
dir milesEq.flt | findstr milesEq.flt
echo === EXPORTS ===
dumpbin /nologo /exports milesEq.flt
echo === HEADERS (machine) ===
dumpbin /nologo /headers milesEq.flt | findstr /C:"machine" /C:"DLL"
echo === IMPORTS (mss32) ===
dumpbin /nologo /dependents milesEq.flt | findstr /I "mss32"
