@echo off

:start
echo MSYS build for Qt
echo =================
echo 1. 32-bit build
echo 2. 64-bit build
echo 3. Debug build
echo X. Exit
echo.

choice /c 123x

if errorlevel 4 goto :eof
if errorlevel 3 (
	set Debug=1
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
call "../Set Common Environment Variables"
if errorlevel 1 goto end

set version=4.8.2
set package=qt-everywhere-opensource-src-%version%.tar.gz
set sourceBaseFolder=Qt%BuildBits%

if "%Debug%" == "1" (
	set sourceBaseFolder=%sourceBaseFolder%-dbg
	set LeakFlags=-L %admBuildDir% -l ADM_core6
	set BuildType=release
) else (
	set BuildType=release
)

set sourceFolder=%sourceBaseFolder%\\%version%
set tarFolder=qt-everywhere-opensource-src-%version%
set curDir=%CD%

if not exist %package% (
	echo.
	echo Downloading
	wget http://releases.qt-project.org/qt4/source/%package%
)

if errorlevel 1 goto end

echo.
echo Preparing
rmdir /s /q "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

if not exist "%devDir%\%sourceBaseFolder%" mkdir "%devDir%\%sourceBaseFolder%"
if errorlevel 1 goto end
mkdir "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

tar xfz "%package%" -C "%devDir%\%sourceFolder%"
if errorlevel 1 goto end

cd "%devDir%\%sourceFolder%"

for /f "delims=" %%a in ('dir /b %tarFolder%') do (
   move "%CD%\%tarFolder%\%%a" "%CD%"
)

if "%Debug%" == "1" (
	copy %admBuildDir%\libADM_core6.dll bin\
	copy %admBuildDir%\pthreadGC2-w%BuildBits%.dll bin\
)

echo.
echo Patching
patch -p0 -i "%curDir%\qmake%BuildBits%.conf.patch"

echo.
echo Configuring
configure -confirm-license -opensource -%BuildType% -system-zlib -no-stl -no-qt3support -no-phonon -no-webkit -no-multimedia -no-declarative -no-style-cleanlooks -no-style-plastique -no-style-motif -no-style-cde -qt-style-windowsxp -qt-style-windowsvista -no-xmlpatterns -nomake demos -nomake examples -platform win32-g++ -mmx -sse -sse2 -3dnow -I %CMAKE_INCLUDE_PATH:;= -I % -L %CMAKE_LIBRARY_PATH:;= -L % %LeakFlags%

if errorlevel 1 goto end
echo.
pause

mingw32-make
if errorlevel 1 goto end

if "%BuildType%" == "debug" set d=d

copy "%devDir%\%sourceFolder%\bin\QtCore%d%4.dll" "%admBuildDir%"
copy "%devDir%\%sourceFolder%\bin\QtGui%d%4.dll" "%admBuildDir%"
copy "%devDir%\%sourceFolder%\bin\QtOpenGL%d%4.dll" "%admBuildDir%"
copy "%devDir%\%sourceFolder%\bin\QtScript%d%4.dll" "%admBuildDir%"
copy "%devDir%\%sourceFolder%\bin\QtScriptTools%d%4.dll" "%admBuildDir%"

goto end

:error
echo Error

:end
pause