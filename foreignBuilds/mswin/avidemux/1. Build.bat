@echo off

set BuildGenerator=MinGW Makefiles

:start
echo MinGW build for Avidemux
echo ========================
echo 1. 32-bit build
echo 2. 64-bit build
echo 3. Debug build
echo 4. Code::Blocks build
echo X. Exit
echo.

choice /c 1234x

if errorlevel 5 goto end
if errorlevel 4 (
	set INCLUDE_MSYS_PATH=false
	set BuildGenerator=CodeBlocks - MinGW Makefiles
	echo.
	echo -- Code::Blocks mode set --
	echo.
	goto :start	)
if errorlevel 3 (
	set DebugFlags=-DCMAKE_BUILD_TYPE=Debug
	echo.
	echo -- Debug mode set --
	echo.
	goto :start	)
if errorlevel 2 (
	set BuildBits=64
	goto :begin	)
if errorlevel 1 (
	set BuildBits=32
	goto :begin	)

:begin
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