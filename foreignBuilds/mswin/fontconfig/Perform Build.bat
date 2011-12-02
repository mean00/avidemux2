@echo off

echo MSYS build for fontconfig
echo =========================
echo 1. 32-bit build
echo 2. 64-bit build
echo X. Exit
echo.

choice /c 12x

if errorlevel 1 set BuildBits=32
if errorlevel 2 set BuildBits=64
if errorlevel 3 goto :eof

verify >nul
call "../Set Common Environment Variables"
if errorlevel 1 goto end

set package=fontconfig-2.8.0.tar.gz
set sourceFolder=fontconfig-2.8.0-%BuildBits%
set tarFolder=fontconfig-2.8.0
set curDir=%CD%

if not exist %package% (
	echo.
	echo Downloading
	wget http://fontconfig.org/release/%package%
)

if errorlevel 1 goto end

echo.
echo Preparing
rm -r -f "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

tar xfz "%package%" -C "%devDir%/%sourceFolder%"
if errorlevel 1 goto end

cd "%devDir%\%sourceFolder%"

for /f "delims=" %%a in ('dir /b %tarFolder%') do (
  move "%CD%\%tarFolder%\%%a" "%CD%"
)

echo.
echo Configuring
sh ./configure --prefix="%usrLocalDir%" --disable-static

if errorlevel 1 goto end
echo.
pause

make install
if errorlevel 1 goto end

del "%usrLocalDir%\etc\fonts\fonts.conf.bak"
copy "%usrLocalDir%\bin\libfontconfig-1.dll" "%admBuildDir%"
xcopy /s/y "%usrLocalDir%\etc\fonts\*.*" "%admBuildDir%\etc\fonts\"

goto end

:error
echo Error

:end
pause