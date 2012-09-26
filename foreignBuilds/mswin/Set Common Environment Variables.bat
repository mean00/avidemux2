set ProgramFiles32=%ProgramFiles%
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" for /D %%d in ("%ProgramFiles(x86)%") do set ProgramFiles32=%%~fsd
set nsisDir=%ProgramFiles32%\NSIS

set devDir=%~d0/Dev
set msysDir=%~d0/Dev/MSYS

if "%BuildBits%" == "32" goto :setVars
if "%BuildBits%" == "64" goto :setVars

echo Error - BuildBits variable not set
goto error

:setVars
set mingwDir=%devDir%/mingw
set usrLocalDir=%msysDir%/local%BuildBits%
set qtDir=%devDir%/Qt%BuildBits%
set CMAKE_INCLUDE_PATH=%usrLocalDir%/include;%mingwDir%/i686-w64-mingw32/include;%mingwDir%/include
set CMAKE_LIBRARY_PATH=%usrLocalDir%/lib
set PKG_CONFIG_PATH=%usrLocalDir%\lib\pkgconfig
set SDLDIR=%usrLocalDir%
set CFLAGS=%CFLAGS% -I%CMAKE_INCLUDE_PATH:;= -I% -L%CMAKE_LIBRARY_PATH:;= -L%
set CXXFLAGS=%CXXFLAGS% -I%CMAKE_INCLUDE_PATH:;= -I% -L%CMAKE_LIBRARY_PATH:;= -L%
set LDFLAGS=%LDFLAGS% -shared-libgcc -L%CMAKE_LIBRARY_PATH:;= -L%

if "%Debug%" EQU "1" (
	set admBuildDir=%devDir%\avidemux_2.6_build%BuildBits%-dbg
	set admSdkBuildDir=%devDir%\avidemux_2.6_build%BuildBits%-dbg_sdk
	set qtDir=%qtDir%-dbg
) else (
	set admBuildDir=%devDir%\avidemux_2.6_build%BuildBits%
	set admSdkBuildDir=%devDir%\avidemux_2.6_build%BuildBits%_sdk
)

set CFLAGS=%CFLAGS% -m%BuildBits%
set CXXFLAGS=%CXXFLAGS% -m%BuildBits%
set LDFLAGS=%LDFLAGS% -m%BuildBits%

if exist "%qtDir%" (
	for /f %%d in ('dir /b /ad /on "%qtDir%"') do set qtVer=%%d
) else (
	echo Qt 4 could not be found at "%qtDir%".  Please download from http://www.trolltech.com
	goto error
)

set qtDir=%qtDir%\%qtVer%

if exist "%devDir%\CMake 2.8\bin" (
	set cmakeDir=%devDir%\CMake 2.8\bin
	goto foundCMake
)

if exist "%ProgramFiles32%\CMake 2.8\bin" (
	set cmakeDir=%ProgramFiles32%\CMake 2.8\bin
) else (
	echo CMake could not be found.  Please download from http://www.cmake.org
	goto error
)

:foundCMake
if not exist "%mingwDir%" (
	echo MinGW could not be found at "%mingwDir%".  Please download from http://www.mingw.org
	goto error
)

if not exist "%nsisDir%" (
	echo NSIS could not be found at "%nsisDir%".  Please download from http://nsis.sourceforge.net
	goto error
)

set PATH=%cmakeDir%;%mingwDir%\bin;%usrLocalDir%\bin;%msysDir%\local-shared\bin;%qtDir%\bin;%devDir%\strawberry\perl\bin;%PATH%

if "%BuildBits%" == "64" set PATH=%mingwDir%\i686-w64-mingw32\lib64;%PATH%

goto end

:error
exit /b 1

:end