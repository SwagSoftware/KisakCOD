@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul
cd /d "%~dp0"
cl /nologo /LD /O2 /MT /DUSE_MSS /DNDEBUG ^
   /I "C:\MilesWinBaby\src\sdk" /I "C:\MilesWinBaby\include" ^
   milesEq.cpp "C:\MilesWinBaby\src\sdk\ribdll.c" ^
   /link /OUT:milesEq.flt "C:\MilesWinBaby\lib\mss32.lib"
echo EXITCODE=%ERRORLEVEL%
