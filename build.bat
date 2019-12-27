@echo off
cd .
psymake all
if %errorlevel% == 0 (
	color a
) else (
	color c
)
pause