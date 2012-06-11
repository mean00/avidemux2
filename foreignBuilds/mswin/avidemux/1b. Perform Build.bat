set INCLUDE_MSYS_PATH=false
set BuildGenerator=CodeBlocks - MinGW Makefiles

if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.workspace" "%SourceDir%\%buildFolder%"

rem ## Core ##
if "%Debug%" EQU "1" (
	if "%BuildBits" EQU "64" (
		set LeakFlags=-DFIND_LEAKS=ON	))

cd "%sourceDir%\%buildCoreFolder%"
cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DBASH_DIR="%msysDir%\bin" %DebugFlags% %LeakFlags% -DUSE_SYSTEM_SPIDERMONKEY=ON -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" ../../avidemux_core

if errorlevel 1 goto error
if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.layout" admCore.layout
pause

rmdir /s /q "%buildDir%\help" 2> NUL
mingw32-make install
if errorlevel 1 goto error

rem ## Qt4 ##
cd "%sourceDir%\%buildQtFolder%"
cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" %DebugFlags% -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" ../../avidemux/qt4

if errorlevel 1 goto error
if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" (
	copy "%curDir%\Tools\avidemux.layout" Avidemux_qt4.layout
	patch -p0 -i "%curDir%\Tools\Avidemux_qt4.cbp.patch"	)
pause

rem ## CLI ##
cd "%sourceDir%\%buildCliFolder%"
cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" %DebugFlags% -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" ../../avidemux/cli

if errorlevel 1 goto error
if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.layout" Avidemux_cli.layout
pause

rem ## GTK ##
rem cd "%sourceDir%\%buildGtkFolder%"
rem cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" %DebugFlags% -DCMAKE_INCLUDE_PATH="%SpiderMonkeySourceDir%" -DCMAKE_LIBRARY_PATH="%SpiderMonkeyLibDir%" ../../avidemux/gtk

rem if errorlevel 1 goto error
rem if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.layout" Avidemux_gtk.layout
rem pause

rem ## Qt4 ##
cd "%sourceDir%\%buildQtFolder%"
mingw32-make install
if errorlevel 1 goto error

rem ## CLI ##
cd "%sourceDir%\%buildCliFolder%"
mingw32-make install
if errorlevel 1 goto error

rem ## GTK ##
rem cd "%sourceDir%\%buildGtkFolder%"
rem mingw32-make install
rem if errorlevel 1 goto error

rem ## Plugins ##
set msysSourceDir=%sourceDir:\=/%
cd "%sourceDir%\%buildPluginFolder%"
cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DAVIDEMUX_SOURCE_DIR="%msysSourceDir%" -DPLUGIN_UI=ALL %DebugFlags% ../../avidemux_plugins

if errorlevel 1 goto error
if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.layout" "%sourceDir%\%buildPluginFolder%\AdmPlugins.layout"
pause

rem ## Plugins ##
cd "%sourceDir%\%buildPluginFolder%"
rmdir /s /q "%buildDir%\plugins" 2> NUL
mingw32-make install
if errorlevel 1 goto error

goto :EOF

:error
exit /b 1