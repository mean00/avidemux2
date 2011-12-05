set ProgramFiles32=%ProgramFiles%
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" for /D %%d in ("%ProgramFiles(x86)%") do set ProgramFiles32=%%~fsd
set nsisDir=%ProgramFiles32%\NSIS

set devDir=E:\Dev
set msysDir=E:/Dev/MSYS

if "%BuildBits%" == "32" goto :setVars
if "%BuildBits%" == "64" goto :setVars

echo Error - BuildBits variable not set
goto error

:setVars
set mingwDir=%devDir%\MinGW64
set usrLocalDir=%msysDir%/local%BuildBits%
set qtDir=%devDir%\Qt%BuildBits%
set CMAKE_INCLUDE_PATH=%usrLocalDir%/include
set CMAKE_LIBRARY_PATH=%usrLocalDir%/lib
set PKG_CONFIG_PATH=%usrLocalDir%\lib\pkgconfig
set SDLDIR=%usrLocalDir%
set CFLAGS=%CFLAGS% -I%CMAKE_INCLUDE_PATH% -L%CMAKE_LIBRARY_PATH%
set CXXFLAGS=%CXXFLAGS% -I%CMAKE_INCLUDE_PATH% -L%CMAKE_LIBRARY_PATH%
set LDFLAGS=%LDFLAGS% -shared-libgcc -shared-libstdc++ -L%CMAKE_LIBRARY_PATH%
set admBuildDir=%devDir%\avidemux_2.6_build%BuildBits%
set admSdkBuildDir=%devDir%\avidemux_2.6_build%BuildBits%_sdk

if "%BuildBits%" == "32" (
	set CFLAGS=%CFLAGS% -m32
	set CXXFLAGS=%CXXFLAGS% -m32
	set LDFLAGS=%LDFLAGS% -m32
)

if exist "%qtDir%" (
	for /f %%d in ('dir /b /ad /on %qtDir%') do set qtVer=%%d
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

if not exist "%CMAKE_INCLUDE_PATH%" (
	echo Include path could not be found at "%CMAKE_INCLUDE_PATH%".
	goto error
)

if not exist "%CMAKE_LIBRARY_PATH%" (
	echo Library path could not be found at "%CMAKE_LIBRARY_PATH%".
	goto error
)

if not exist "%nsisDir%" (
	echo NSIS could not be found at "%nsisDir%".  Please download from http://nsis.sourceforge.net
	goto error
)

set PATH=%cmakeDir%;%mingwDir%\bin;%usrLocalDir%\bin;%msysDir%\local-shared\bin;%qtDir%\bin;c:\strawberry\perl\bin;%PATH%

goto end

:error
exit /b 1

:end