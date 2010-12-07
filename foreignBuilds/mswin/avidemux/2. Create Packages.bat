@echo off

echo Package Avidemux
echo ================
echo 1. 32-bit package
echo 2. 64-bit package
echo X. Exit
echo.

choice /c 12x

if errorlevel 1 set BuildBits=32
if errorlevel 2 set BuildBits=64
if errorlevel 3 goto end

verify >nul
echo.

set curDir=%CD%

call "Set Avidemux Environment Variables"
if errorlevel 1 goto error

cd "%curDir%"
call "2a. Update Notes.bat"
if errorlevel 1 goto error

cd "%curDir%\Tools"
call "Get Revision Number"
cd ..

set packageDir=%CD%\%revisionNo% [%BuildBits%-bit]
mkdir "%packageDir%"

cd "%curDir%"
call "2b. Package SDK.bat"
if errorlevel 1 goto error

cd "%curDir%"
call "2c. Package Build.bat"
if errorlevel 1 goto error

goto end

:error
exit /b 1

:end
pause