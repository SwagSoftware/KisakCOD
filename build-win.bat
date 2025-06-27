@echo off
(
scripts\mksln.bat Debug
cmake --build .\build\
echo %cd%
)