mkdir "%sdkBuildDir%\lib%BuildBits%"
move "%buildDir%\lib\*.a" "%sdkBuildDir%\lib%BuildBits%"
del /s "%buildDir%\*.a"

if "%BuildBits%" == "32" (
	mkdir "%sdkBuildDir%\plugin-examples"
	cd "%sdkBuildDir%\plugin-examples"
	copy "%curDir%\Tools\Build Plugins.bat" "%sdkBuildDir%\plugin-examples"

	mkdir "audioDecoders"
	mkdir "audioDecoders\ac3"
	xcopy /s "%sourceDir%\plugins\ADM_audioDecoders\ADM_ad_ac3" "audioDecoders\ac3"

	mkdir "audioDevices"
	mkdir "audioDevices\win32"
	xcopy /s "%sourceDir%\plugins\ADM_audioDevices\Win32" "audioDevices\win32"

	mkdir "audioEncoders"
	mkdir "audioEncoders\pcm"
	xcopy /s "%sourceDir%\plugins\ADM_audioEncoders\pcm" "audioEncoders\pcm"

	mkdir "videoEncoders"
	mkdir "videoEncoders\mpeg2enc"
	mkdir "videoEncoders\mpeg2enc\common"
	xcopy /s "%sourceDir%\plugins\ADM_videoEncoder\ADM_vidEnc_mpeg2enc" "videoEncoders\mpeg2enc"
	xcopy /s "%sourceDir%\plugins\ADM_videoEncoder\common" "videoEncoders\mpeg2enc\common"
	xcopy /s "%usrLocalDir%\lib\libxml2.dll.a" "videoEncoders\mpeg2enc"
	xcopy /s "%usrLocalDir%\include\libxml2" "videoEncoders\mpeg2enc"
	rmdir /s/q "videoEncoders\mpeg2enc\mpeg2enc\altivec"

	mkdir "videoFilters"
	mkdir "videoFilters\fade"
	xcopy /s "%sourceDir%\plugins\ADM_videoFilters\Fade" "videoFilters\fade"

	del /s CMakeLists.txt

	mkdir "%sdkBuildDir%\include"
	mkdir "%sdkBuildDir%\include\ADM_encoder"
	mkdir "%sdkBuildDir%\include\libavutil"
	cd "%sdkBuildDir%\include"

	copy "%sourceDir%\%buildFolder%\config\ADM_coreConfig.h"
	copy "%sourceDir%\%buildFolder%\config\libavutil\avconfig.h" libavutil
	copy "%sourceDir%\avidemux\ADM_audiocodec\ADM_ad_plugin.h"
	copy "%sourceDir%\avidemux\ADM_audiocodec\ADM_audiocodec.h"
	copy "%sourceDir%\avidemux\ADM_audiodevice\ADM_audiodevice.h"
	copy "%sourceDir%\avidemux\ADM_audiodevice\ADM_audioDeviceInternal.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_assert.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_clock.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_cpucap.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_default.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_dynamicLoading.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_files.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_inttype.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_mangle.h"
	copy "%sourceDir%\avidemux\ADM_core\include\ADM_misc.h"
	copy "%sourceDir%\avidemux\ADM_coreAudio\include\ADM_audioCodecEnum.h"
	copy "%sourceDir%\avidemux\ADM_coreAudio\include\ADM_audiodef.h"
	copy "%sourceDir%\avidemux\ADM_coreAudio\include\ADM_audioFilter.h"
	copy "%sourceDir%\avidemux\ADM_coreAudio\include\ADM_coreAudio.h"
	copy "%sourceDir%\avidemux\ADM_coreAudio\include\audioencoder.h"
	copy "%sourceDir%\avidemux\ADM_coreAudio\include\audioencoderInternal.h"
	copy "%sourceDir%\avidemux\ADM_coreImage\include\ADM_confCouple.h"
	copy "%sourceDir%\avidemux\ADM_coreImage\include\ADM_image.h"
	copy "%sourceDir%\avidemux\ADM_coreImage\include\ADM_rgb.h"
	copy "%sourceDir%\avidemux\ADM_coreImage\include\ADM_videoFilter.h"
	copy "%sourceDir%\avidemux\ADM_coreImage\include\ADM_videoFilterCache.h"
	copy "%sourceDir%\avidemux\ADM_coreImage\include\ADM_videoFilterDynamic.h"
	copy "%sourceDir%\avidemux\ADM_coreImage\include\ADM_videoFilter_iface.h"
	copy "%sourceDir%\avidemux\ADM_coreImage\include\ADM_videoFilter_internal.h"
	copy "%sourceDir%\avidemux\ADM_coreUI\include\DIA_coreToolkit.h"
	copy "%sourceDir%\avidemux\ADM_coreUI\include\DIA_factory.h"
	copy "%sourceDir%\avidemux\ADM_coreUI\include\DIA_uiTypes.h"
	copy "%sourceDir%\avidemux\ADM_encoder\ADM_vidEncode.hxx" ADM_encoder
	copy "%sourceDir%\avidemux\ADM_libraries\ffmpeg\libavutil\pixfmt.h" libavutil
	copy "%sourceDir%\avidemux\ADM_plugin\ADM_vidEnc_plugin.h"
	copy "%sourceDir%\avidemux\ADM_plugin\ADM_plugin_translate.h"

	cd "%sourceDir%\po"
	svn revert -R .
	sh qt_update_pro.sh
	echo ./avidemux_blank.ts >> avidemux.pro
	lupdate avidemux.pro

	mkdir "%sdkBuildDir%\i18n"
	mkdir "%sdkBuildDir%\i18n\qt4"
	mkdir "%sdkBuildDir%\i18n\gtk"

	copy *.ts "%sdkBuildDir%\i18n\qt4"
	copy qt_filter_context.xslt "%sdkBuildDir%\i18n\qt4\qt_filter_context.xsl"
	copy "%curDir%\Tools\Build Qt Translations.bat" "%sdkBuildDir%\i18n\qt4\Build Translations.bat"

	del avidemux_blank.ts

	sh update_pot.bash
	copy avidemux.pot "%sdkBuildDir%\i18n\gtk"
	for %%A in (*.po) do msgmerge %%A avidemux.pot -o "%sdkBuildDir%\i18n\gtk\%%A"

	svn revert -R .
)