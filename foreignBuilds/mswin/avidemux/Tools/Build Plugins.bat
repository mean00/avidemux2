@echo off

if "%1" == "" (
	echo Usage: "Build Plugins.bat" [MinGW directory]
	echo e.g. "Build Plugins.bat" C:\MinGW
	goto :EOF
)

set curDir=%CD%
set includeDir="%curDir%\..\include"
set libDir="%curDir%\..\lib"
set ar="%1\bin\ar"
set gcc="%1\bin\gcc" -O3 -DNDEBUG
set gxx="%1\bin\g++" -O3 -DNDEBUG -Wl,-enable-auto-import -Wl,-s

echo ** AC-3 Audio Decoder **

cd audioDecoders\ac3\ADM_liba52
%gcc% -c *.c -I%includeDir% -L%libDir% -lADM_core.dll
%ar% rs libADM_liba52.a *.o
cd ..
%gxx% -shared *.cpp -o libADM_ad_a52.dll -I%includeDir% -LADM_liba52 -lADM_liba52 -L%libDir% -lADM_core.dll
cd %curDir%

echo.
echo ** Win32 Audio Device **

cd audioDevices\win32
%gxx% -shared *.cpp -o libADM_av_win32.dll -I%includeDir% -L%libDir% -lADM_core.dll -lADM_coreAudio.dll -lwinmm
cd %curDir%

echo.
echo ** PCM Audio Encoder **

cd audioEncoders\pcm
%gxx% -shared *.cpp -o libADM_ae_pcm.dll -I%includeDir% -L%libDir% -lADM_core.dll -lADM_coreAudio.dll
cd %curDir%

echo.
echo ** mpeg2enc Video Encoder **

cd videoEncoders\mpeg2enc\mpeg2enc
%gcc% -c *.c *.cc -I. -I%includeDir% -DHAVE_CONFIG_H -DHAVE_X86CPU
%ar% rs libmpeg2enc.a *.o
cd ..
%gcc% -c *.c -I%includeDir%
%gxx% -shared *.o *.cpp common\pluginOptions\*.cpp common\xvidRateCtl\*.cpp -o libADM_vidEnc_mpeg2enc.dll -I%includeDir% -I. -I.\mpeg2enc -I.\common\pluginOptions -I.\common\xvidRateCtl -DMPEG1_PLUGIN_CONFIG_DIR=\"mpeg2enc/mpeg-1\" -DMPEG2_PLUGIN_CONFIG_DIR=\"mpeg2enc/mpeg-2\" -L. -lxml2.dll -Lmpeg2enc -lmpeg2enc -L%libDir% -lADM_coreUI.dll -lADM_core.dll
cd %curDir%

echo.
echo ** Fade Video Filter **

cd videoFilters\fade
%gxx% -shared *.cpp -o libADM_vf_fade.dll -I%includeDir% -L%libDir% -lADM_coreImage.dll -lADM_core.dll -lADM_coreUI.dll
cd %curDir%