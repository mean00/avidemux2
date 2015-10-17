@echo off

if "%1" == "" (
	echo Usage: "Build Translations.bat" [Qt directory]
	echo e.g. "Build Translations.bat" C:\Qt\4.5.3
	goto :EOF
)

for %%A in (avidemux*.ts) do (
	echo.
	echo Building %%~nA
	echo.
	"%1\bin\lrelease" "%%A" -qm "%%~nA.qm"
	del "%%~nA.xml"
)

for %%A in (qt*.ts) do (
	echo.
	echo Building %%~nA
	echo.
	"%1\bin\lrelease" "%%A"
)
