@echo off

echo MSYS build for FriBidi
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

set package=fribidi-0.19.4.tar.bz2
set sourceFolder=fribidi-0.19.4-%BuildBits%
set tarFolder=fribidi-0.19.4
set curDir=%CD%
set PATH=%msysDir%\bin;%PATH%

if not exist %package% (
	echo.
	echo Downloading
	wget http://fribidi.org/download/%package%
)

if errorlevel 1 goto end

echo.
echo Preparing
rm -r -f "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

cd "%devDir%\%sourceFolder%"

set SevenZipDir=%ProgramFiles%\7-zip

"%SevenZipDir%"\7z x "%curDir%\%package%"
if errorlevel 1 goto end

"%SevenZipDir%"\7z x "%tarFolder%.tar"
if errorlevel 1 goto end

for /f "delims=" %%a in ('dir /b %tarFolder%') do (
  move "%CD%\%tarFolder%\%%a" "%CD%"
)

echo.
echo Patching
patch -p0 -i "%curDir%\Makefile.in.patch"

echo.
echo Configuring
sh ./configure --prefix="%usrLocalDir%" --disable-static

if errorlevel 1 goto end
echo.
pause

make install
if errorlevel 1 goto end

strip "%usrLocalDir%\bin\libfribidi-0.dll"
copy "%usrLocalDir%\bin\libfribidi-0.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause