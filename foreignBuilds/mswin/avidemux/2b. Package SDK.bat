set baseFile=avidemux_sdk_2.5_r%revisionNo%_win%BuildBits%
set zipFile=%baseFile%.zip

cd "%sdkBuildDir%"
zip -r "%packageDir%\%zipFile%" *
advzip -z -4 "%packageDir%\%zipFile%"