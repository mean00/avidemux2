@echo off

echo MSYS build for twolame
echo ======================
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

set version=0.3.13
set package=twolame-%version%.tar.gz
set sourceFolder=twolame-%version%-%BuildBits%
set tarFolder=twolame-%version%
set curDir=%CD%
set PATH=%PATH%;%msysDir%\bin

if not exist %package% (
	echo.
	echo Downloading
	wget http://sourceforge.net/projects/twolame/files/twolame/%version%/%package%/download
)

if errorlevel 1 goto end

echo.
echo Preparing
rm -r -f "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

tar xfz "%package%" -C "%devDir%\\%sourceFolder%"
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

make CFLAGS="-O3 -DLIBTWOLAME_DLL_EXPORTS" LDFLAGS="-no-undefined" install-strip
if errorlevel 1 goto end

copy "%usrLocalDir%\bin\libtwolame-0.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause