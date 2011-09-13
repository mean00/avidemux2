set zipFile=avidemux_sdk_2.6_r%revisionNo%_win.zip

if "%BuildBits%" == "32" (
	set sdk32BuildDir=%sdkBuildDir%
	set sdk64BuildDir=%sdkBuildDir:build32=build64%
)

if "%BuildBits%" == "64" (
	set sdk32BuildDir=%sdkBuildDir:build64=build32%
	set sdk64BuildDir=%sdkBuildDir%
)

cd "%sdk32BuildDir%"
zip -r "%packageDir%\%zipFile%" *

cd "%sdk64BuildDir%"
zip -r "%packageDir%\%zipFile%" *

advzip -z -4 "%packageDir%\%zipFile%"