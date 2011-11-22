if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.workspace" "%SourceDir%\%buildFolder%"

rem ## Core ##
cd "%sourceDir%\%buildCoreFolder%"
cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DBASH_DIR="%msysDir%\bin" %DebugFlags% ../../avidemux_core

if errorlevel 1 goto error
if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.layout" admCore.layout
pause

mingw32-make install
if errorlevel 1 goto error

rem ## Qt4 ##
cd "%sourceDir%\%buildQtFolder%"
cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" %DebugFlags% ../../avidemux/qt4

if errorlevel 1 goto error
if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" (
	copy "%curDir%\Tools\avidemux.layout" Avidemux_qt4.layout
	patch -p0 -i "%curDir%\Tools\Avidemux_qt4.cbp.patch"	)
pause

mingw32-make install
if errorlevel 1 goto error

rem ## CLI ##
cd "%sourceDir%\%buildCliFolder%"
cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" %DebugFlags% ../../avidemux/cli

if errorlevel 1 goto error
if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.layout" Avidemux_cli.layout
pause

mingw32-make install
if errorlevel 1 goto error

rem ## GTK ##
cd "%sourceDir%\%buildGtkFolder%"
rem cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" %DebugFlags% ../../avidemux/gtk

if errorlevel 1 goto error
rem if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.layout" Avidemux_gtk.layout
rem pause

rem mingw32-make install
if errorlevel 1 goto error

rem ## Plugins ##
set msysSourceDir=%sourceDir:\=/%
cd "%sourceDir%\%buildPluginFolder%"
cmake -G"%BuildGenerator%" -DCMAKE_INSTALL_PREFIX="%buildDir%" -DAVIDEMUX_SOURCE_DIR="%msysSourceDir%" -DPLUGIN_UI=ALL %DebugFlags% ../../avidemux_plugins

if errorlevel 1 goto error
if "%BuildGenerator%" == "CodeBlocks - MinGW Makefiles" copy "%curDir%\Tools\avidemux.layout" "%sourceDir%\%buildPluginFolder%\AdmPlugins.layout"
pause

rmdir /s /q "%buildDir%\plugins" 2> NUL
mingw32-make install
if errorlevel 1 goto error

goto :EOF

:error
exit /b 1