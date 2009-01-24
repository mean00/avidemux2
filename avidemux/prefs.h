#ifndef _preferences_h_
#define _preferences_h_

#include "ADM_default.h"

#define RC_OK     1
#define RC_FAILED 0

// <prefs_gen>
typedef enum {
	CODECS_SVCD_ENCTYPE,
	CODECS_SVCD_BITRATE,
	CODECS_SVCD_QUANTIZER,
	CODECS_SVCD_FINALSIZE,
	CODECS_SVCD_INTERLACED,
	CODECS_SVCD_BFF,
	CODECS_SVCD_WIDESCREEN,
	CODECS_SVCD_MATRIX,
	CODECS_SVCD_GOPSIZE,
	CODECS_SVCD_MAXBITRATE,
	CODECS_DVD_ENCTYPE,
	CODECS_DVD_BITRATE,
	CODECS_DVD_QUANTIZER,
	CODECS_DVD_FINALSIZE,
	CODECS_DVD_INTERLACED,
	CODECS_DVD_BFF,
	CODECS_DVD_WIDESCREEN,
	CODECS_DVD_MATRIX,
	CODECS_DVD_GOPSIZE,
	CODECS_DVD_MAXBITRATE,
	CODECS_XVID_ENCTYPE,
	CODECS_XVID_QUANTIZER,
	CODECS_XVID_BITRATE,
	CODECS_XVID_FINALSIZE,
	CODECS_PREFERREDCODEC,
	FILTERS_SUBTITLE_FONTNAME,
	FILTERS_SUBTITLE_CHARSET,
	FILTERS_SUBTITLE_FONTSIZE,
	FILTERS_SUBTITLE_YPERCENT,
	FILTERS_SUBTITLE_UPERCENT,
	FILTERS_SUBTITLE_VPERCENT,
	FILTERS_SUBTITLE_SELFADJUSTABLE,
	FILTERS_SUBTITLE_USEBACKGROUNDCOLOR,
	SETTINGS_MPEGSPLIT,
	DEVICE_AUDIODEVICE,
	DEVICE_AUDIO_ALSA_DEVICE,
	DEVICE_VIDEODEVICE,
	DEFAULT_POSTPROC_TYPE,
	DEFAULT_POSTPROC_VALUE,
	LASTFILES_FILE1,
	LASTFILES_FILE2,
	LASTFILES_FILE3,
	LASTFILES_FILE4,
	LASTDIR_READ,
	LASTDIR_WRITE,
	LAME_CLI,
	PIPE_CMD,
	PIPE_PARAM,
	LAME_PATH,
	TOOLAME_PATH,
	LVEMUX_PATH,
	REQUANT_PATH,
	MESSAGE_LEVEL,
	FEATURE_SWAP_IF_A_GREATER_THAN_B,
	FEATURE_SVCDRES_PREFEREDSOURCERATIO,
	FEATURE_SAVEPREFSONEXIT,
	FEATURE_IGNORESAVEDMARKERS,
	FEATURE_DISABLE_NUV_RESYNC,
	FEATURE_TRYAUTOIDX,
	FEATURE_USE_ODML,
	FEATURE_USE_SYSTRAY,
	FEATURE_REUSE_2PASS_LOG,
	FEATURE_AUDIOBAR_USES_MASTER,
	FEATURE_THREADING_LAVC,
	FEATURE_THREADING_X264,
	FEATURE_THREADING_XVID,
	FEATURE_CPU_CAPS,
	FEATURE_MPEG_NO_LIMIT,
	FEATURE_AUTO_BUILDMAP,
	FEATURE_AUTO_REBUILDINDEX,
	FEATURE_AUTO_UNPACK,
	DOWNMIXING_PROLOGIC,
	FILTERS_AUTOLOAD_PATH,
	FILTERS_AUTOLOAD_ACTIVE,
	FEATURE_ALTERNATE_MP3_TAG,
	FEATURE_GLOBAL_GLYPH_ACTIVE,
	FEATURE_GLOBAL_GLYPH_NAME,
	PRIORITY_ENCODING,
	PRIORITY_INDEXING,
	PRIORITY_PLAYBACK
} options;
// </prefs_gen>

class preferences {
	private:
		char *internal_lastfiles[5];
		int save_xml_to_file();
	public:
		preferences();
		~preferences();
		int load();
		int save();
		int get(options option, unsigned int *val);
		int get(options option,          int *val);
		int get(options option, unsigned long *val);
		int get(options option,          long *val);
		int get(options option, float *val);
		int get(options option, char **val);
                int get(options option, ADM_filename **val);
		int get(options option, uint8_t *val);
		int get(options option, uint16_t *val);
		const char * get_str_min(options option);
		const char * get_str_max(options option);
		// handled by compiler: const uint8_t is full handled by any 
		//    method that can handle const unsigned int
		// int set(options option, const uint8_t val);
		// int set(options option, const uint16_t val);
		int set(options option, const unsigned int val);
		int set(options option, const          int val);
		int set(options option, const unsigned long val);
		int set(options option, const          long val);
		int set(options option, const          float val);
		int set(options option, const          char * val);
                int set(options option, const ADM_filename * val);
		int set_lastfile(const char* file);
		const char **get_lastfiles(void);
};

extern preferences *prefs;
int initPrefs(  void );
int destroyPrefs(  void );
#endif
