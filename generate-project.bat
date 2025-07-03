@echo off
(
scripts\mksln_mp.bat Debug
scripts\mksln_sp.bat Debug
scripts\mksln_dedi.bat Debug
echo %cd%
)