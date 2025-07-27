@echo off
(
scripts\mksln.bat Debug
cmake --build .\build\ --target "KisakCOD-mp"
cmake --build .\build\ --target "KisakCOD-dedi"
echo %cd%
)