@echo off

:start
echo MinGW build for Avidemux
echo ========================
echo 1. 32-bit build
echo 2. 64-bit build
echo 3. Debug build
echo X. Exit
echo.

choice /c 123x

if errorlevel 1 set BuildBits=32
if errorlevel 2 set BuildBits=64
if errorlevel 3 (
	set DebugFlags=-DCMAKE_BUILD_TYPE=Debug
	echo.
	echo -- Debug mode set --
	echo.
	goto :start	)

if errorlevel 4 goto end

verify >nul
echo.

set curDir=%CD%

call "Set Avidemux Environment Variables"
if errorlevel 1 goto end

cd "%curDir%"
call "1a. Pre-build"
if errorlevel 1 goto end

cd "%curDir%"
call "1b. Perform Build"
if errorlevel 1 goto end

cd "%curDir%"
call "1c. Post-build"
if errorlevel 1 goto end

echo Finished!

:end
pause