set baseFile=avidemux_2.6_r%revisionNo%_win%BuildBits%
set zipFile=%baseFile%.zip

copy "%buildDir%\Change Log.html" "%packageDir%"
move "Tools\Package Notes [%BuildBits%].html" "%packageDir%\Package Notes.html"

cd %buildDir%
rem if "%BuildBits%" == "32" (
rem	echo -- Generating GTK+ Installer --
rem	"%nsisDir%\makensis" /V2 /NOCD /DINST_GTK /DBUILD_BITS=%BuildBits% /DNSIDIR="%curDir%\..\..\..\avidemux\wininstaller" /DEXEDIR="%packageDir%" "%curDir%\..\..\..\avidemux\wininstaller\avidemux.nsi"
rem )

echo -- Generating Qt Installer --
"%nsisDir%\makensis" /V2 /NOCD /DINST_QT /DBUILD_BITS=%BuildBits% /DNSIDIR="%curDir%\..\..\..\avidemux\wininstaller" /DEXEDIR="%packageDir%" "%curDir%\..\..\..\avidemux\wininstaller\avidemux.nsi"

mkdir "%packageDir%\temp"
cd "%packageDir%\temp"
"%SevenZipDir%\7z" x "%packageDir%\%baseFile%.exe"
rmdir /s/q $PLUGINSDIR
mkdir scripts
move $_OUTDIR\auto scripts
move $_OUTDIR\video scripts
mkdir etc\fonts\conf.d
copy "%buildDir%\etc\fonts\conf.d" etc\fonts\conf.d\
move $_OUTDIR\conf.avail etc\fonts
rmdir /s/q $_OUTDIR
zip -r "%packageDir%\%zipFile%" *
cd %curDir%
advzip -z -4 "%packageDir%\%zipFile%"
rmdir /s/q "%packageDir%\temp"