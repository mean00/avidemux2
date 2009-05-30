/***************************************************************************
                          adm_encConfig.cpp  -  description
                             -------------------
    begin                : Thu Dec 26 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_lavcodec.h"

#include "ADM_assert.h"
#include "fourcc.h"
#include "ADM_quota.h"
#include "ADM_editor/ADM_edit.hxx"
#include "DIA_coreToolkit.h"

#include "prefs.h"
#include "ADM_vidEncode.hxx"
#include "ADM_videoFilter.h"

#include "avi_vars.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_ENCODER
#include "ADM_osSupport/ADM_debug.h"

extern struct COMPRES_PARAMS *AllVideoCodec;
extern int AllVideoCodecCount;
extern uint8_t DIA_videoCodec(int *codecIndex);
extern void UI_setVideoCodec(int i);
extern void getMainWindowHandles(long int *handle, long int *nativeHandle);

// Some static stuff
void setVideoEncoderSettings(COMPRESSION_MODE mode, uint32_t param, uint32_t extraConf, uint8_t *extraData);
static void encoderPrint(void);


#include "adm_encoder.h"

#ifdef USE_XVID_4
#include "ADM_codecs/ADM_xvid4.h"
#include "ADM_codecs/ADM_xvid4param.h"
#include "adm_encXvid4.h"
#endif

#ifdef USE_XX_XVID
#include "ADM_codecs/ADM_xvid.h"
#include "adm_encxvid.h"
#include "xvid.h"
#endif

#include "ADM_codecs/ADM_ffmpeg.h"
#include "adm_encffmpeg.h"

#include "ADM_codecs/ADM_mjpegEncode.h"
#include "adm_encmjpeg.h"
#include "adm_encCopy.h"
#include "adm_encyv12.h"
#include "adm_encConfig.h"
#include "ADM_pluginLoad.h"
#include "ADM_externalEncoder.h"


/*
  	Codec settings here
*/
#include "ADM_encCodecDesc.h"

SelectCodecType currentCodecType = CodecCopy;
int currentCodecIndex = 0;

uint8_t mk_hex (uint8_t a, uint8_t b);

/**
 *      \fn getCodecParamFromTag
 *      \brief Returns the codec descriptor from its tag. It is usefull only for internal filters.
 */
static COMPRES_PARAMS* getCodecParamFromTag(    SelectCodecType tag)
{
    if(tag==CodecExternal) return NULL;
    for(int i=0;i<AllVideoCodecCount;i++)
      {
        COMPRES_PARAMS *r=&(AllVideoCodec[i]);
        if(r->codec==tag) return r;
      }
    return NULL;

}
CodecFamilty videoCodecGetFamily(void)
{
	if (currentCodecType == CodecXVCD || currentCodecType == CodecXSVCD || currentCodecType == CodecXDVD)
		return CodecFamilyXVCD;
	if (currentCodecType == CodecVCD || currentCodecType == CodecSVCD || currentCodecType == CodecDVD || currentCodecType == CodecRequant)
		return CodecFamilyMpeg;

	return CodecFamilyAVI;
}

uint8_t videoCodecSetFinalSize(uint32_t size)
{
	COMPRES_PARAMS *mode = &AllVideoCodec[currentCodecIndex];

	mode->finalsize = size;

	if (mode->capabilities & ADM_ENC_CAP_2PASS)
		mode->mode = COMPRESS_2PASS;

	return 1;
}
/*
	We return 2 things :
		The codec conf : i.e. mode process and the extraData size
*/
uint32_t videoProcessMode(void)
{
	return (currentCodecType != CodecCopy);
}

/*
	Ultimately that will be dispatched on a per codec
	stuff and will be merged with xml stuff to allow
	save config / load config
*/
int videoCodecConfigureAVI(char *cmdString, uint32_t optSize, uint8_t * opt)
{
#define UNSET_COMPRESSION_MODE (COMPRESSION_MODE)0xff
#define NO_COMPRESSION_MODE    (COMPRESSION_MODE)0xfe

	COMPRES_PARAMS *param;
	int iparam, equal = 0xfff;
	COMPRESSION_MODE compmode = UNSET_COMPRESSION_MODE;
	char *cs = cmdString;
	char *go = cmdString;

	while (*go)
	{
		// search the =
		for (unsigned int i = 0; i < strlen (cs); i++)
		{
			if (*(cs + i) == '=')
			{
				equal = i;
				break;
			}
		}

		if (equal == 0xfff)
		{
			printf ("Invalid video conf args.\n");
			return 0;
		}

		go = cs + equal + 1;
		*(cs + equal) = 0;
		iparam = atoi (cs + equal + 1);
		printf ("Codec conf is %s\n", cs);

		// search the codec
		if (!strcasecmp (cs, "aq"))
		{
			compmode = COMPRESS_AQ;
			aprintf ("aq Mode\n");
		}

		if (!strcasecmp (cs, "cq"))
		{
			compmode = COMPRESS_CQ;
			aprintf ("cq Mode\n");
		}

		if (!strcasecmp (cs, "cbr"))
		{
			compmode = COMPRESS_CBR;
			//iparam*=1000;
			aprintf ("cbr Mode\n");
		}

		if (!strcasecmp (cs, "2pass"))
		{
			compmode = COMPRESS_2PASS;
			aprintf ("2pass\n");
		}

		if (!strcasecmp (cs, "2passbitrate"))
		{
			compmode = COMPRESS_2PASS_BITRATE;
			aprintf ("2pass bitrate\n");
		}

		if (!strcasecmp (cs, "follow"))
		{
			compmode = COMPRESS_SAME;
			aprintf ("Follow mode\n");
		}

		// search for other options
		if (!strcasecmp (cs, "mbr"))
		{
			compmode = NO_COMPRESSION_MODE;
			iparam = (iparam * 1000) >> 3;

			switch (currentCodecType)
			{
			case CodecSVCD:
				SVCDExtra.maxBitrate = (int) iparam;
				break;
			case CodecDVD:
				DVDExtra.maxBitrate = (int) iparam;
				break;
			default:
				break;
			}
		}

		if (!strcasecmp (cs, "matrix"))
		{
			compmode = NO_COMPRESSION_MODE;

			switch (currentCodecType)
			{
			case CodecSVCD:
				SVCDExtra.user_matrix = (int) iparam;
				break;
			case CodecDVD:
				DVDExtra.user_matrix = (int) iparam;
				break;
			default:
				break;
			}
		}

		if (!strcmp (cs, "ws"))
		{
			switch (currentCodecType)
			{
			case CodecDVD:
				DVDExtra.widescreen = 1;
			case CodecSVCD:
				SVCDExtra.widescreen = 1;
				break;
			default:
				break;
			}
		}

		if (compmode == UNSET_COMPRESSION_MODE)
		{
			printf ("\n ***** Unknown mode for video codec (%s)\n", cmdString);
			return 0;
		}

		if (compmode != NO_COMPRESSION_MODE)
		{
			aprintf ("param:%d\n", iparam);
			setVideoEncoderSettings (compmode, iparam, optSize, opt);
		}

		// find next option
		for (; *go != '\0'; go++)
		{
			if (*go == ',')
			{
				cs = go + 1;
				break;
			}
		}
	}

	return 1;
}

int videoCodecConfigure(char *cmdString, uint32_t optionSize, uint8_t * option)
{
	if (!cmdString)
		return 0;

	CodecFamilty family = videoCodecGetFamily();

	switch (family)
	{
	case CodecFamilyAVI:
	case CodecFamilyXVCD:
	case CodecFamilyMpeg:
		return videoCodecConfigureAVI(cmdString, optionSize, option);
		break;
		/*			case CodecFamilyMpeg :
		videoCodecConfigureMpeg(cmdString);
		break;
		*/
	default:
		printf ("This codec family does not accept paramaters\n");
		return 0;
	}

	return 0;
}

// Used to know the # of menu entries
int encoderGetEncoderCount(void)
{
	return AllVideoCodecCount;
}

// Return the name of the encoder #i, as displayer by a menu/combo box
const char *encoderGetIndexedName(uint32_t i)
{
	int nb = encoderGetEncoderCount();

	ADM_assert(i < nb);

	return AllVideoCodec[i].menuName;
}

void videoCodecChanged(int newCodecIndex)
{
	ADM_assert(newCodecIndex < encoderGetEncoderCount());

	currentCodecType = AllVideoCodec[newCodecIndex].codec;
	currentCodecIndex = newCodecIndex;
}

void encoderPrint(void)
{
	UI_setVideoCodec(currentCodecIndex);
}

void saveEncoderConfig(void)
{
	prefs->set (CODECS_PREFERREDCODEC, AllVideoCodec[currentCodecIndex].tagName);
}

SelectCodecType videoCodecGetType(void)
{
	return currentCodecType;
}

int videoCodecGetIndex(void)
{
	return currentCodecIndex;
}

int videoCodecSelectByName(const char *name)
{
	int nb = encoderGetEncoderCount();

	for (uint32_t i = 0; i < nb; i++)
	{
		if (!strcasecmp(name, AllVideoCodec[i].tagName))
		{
			printf ("\n Codec %s found\n", name);
			videoCodecSetCodec(i);

			return 1;
		}
	}

	printf ("\n Mmmm Select codec by name failed...(%s).\n", name);
	printf (" Available codec : %"LU"\n", sizeof (AllVideoCodec) / sizeof (COMPRES_PARAMS));

	for (uint32_t i = 0; i < nb; i++)
		printf ("%s:%s\n", AllVideoCodec[i].tagName, AllVideoCodec[i].descriptor);

	return 0;
}

int videoCodecPluginSelectByGuid(const char *guid)
{
	int encoderCount = encoderGetEncoderCount();

	for (int i = 0; i < encoderCount; i++)
	{
		if (AllVideoCodec[i].codec == CodecExternal)
		{
			ADM_vidEnc_plugin *plugin = getVideoEncoderPlugin(AllVideoCodec[i].extra_param);
			const char* encoderGuid = plugin->getEncoderGuid(plugin->encoderId);

			if (strcmp(encoderGuid, guid) == 0)
			{
				printf ("Codec plugin %s (%s) found\n", plugin->getEncoderName(plugin->encoderId), encoderGuid);
				videoCodecSetCodec(i);

				return 1;
			}
		}
	}

	return 0;
}

const char* videoCodecPluginGetGuid(void)
{
	ADM_vidEnc_plugin *plugin = getVideoEncoderPlugin(AllVideoCodec[currentCodecIndex].extra_param);

	return plugin->getEncoderGuid(plugin->encoderId);
}

const char *videoCodecGetMode(void)
{
	uint8_t *data;
	uint32_t nbData = 0;
	static char string[90];
	COMPRES_PARAMS *mode = &AllVideoCodec[currentCodecIndex];

	switch (mode->mode)
	{
	case COMPRESS_AQ:
		sprintf (string, "AQ=%d", mode->qz);
		break;
	case COMPRESS_CQ:
		sprintf (string, "CQ=%d", mode->qz);
		break;
	case COMPRESS_CBR:
		sprintf (string, "CBR=%d", mode->bitrate);
		break;
	case COMPRESS_2PASS:
		sprintf (string, "2PASS=%d", mode->finalsize);
		break;
	case COMPRESS_2PASS_BITRATE:
		sprintf (string, "2PASSBITRATE=%d", mode->avg_bitrate);
		break;
	case COMPRESS_SAME:
		sprintf (string, "FOLLOW=0");
		break;
	default:
		ADM_assert (0);
	}

	return string;
}

const char *videoCodecGetName(void)
{
	return AllVideoCodec[currentCodecIndex].tagName;
}

///
///  Prompt for a codec and allow configuration
///
void videoCodecSelect(void)
{
  DIA_videoCodec(&currentCodecIndex);
  encoderPrint ();
  // HERE UI_PrintCurrentVCodec( (currentCodecType))

}

void videoCodecSetCodec(int codecIndex)
{
	currentCodecIndex = codecIndex;
	currentCodecType = AllVideoCodec[codecIndex].codec;
	encoderPrint();
}

void videoCodecConfigureUI(int codecIndex)
{
	if (codecIndex == -1)
		codecIndex = currentCodecIndex;

	COMPRES_PARAMS *param = &AllVideoCodec[codecIndex];

	if (AllVideoCodec[codecIndex].codec == CodecExternal)
	{
		ADM_vidEnc_plugin *plugin = getVideoEncoderPlugin(param->extra_param);
		vidEncOptions encodeOptions;
		vidEncVideoProperties properties;
		vidEncConfigParameters configParameters;

		memset(&properties, 0, sizeof(vidEncVideoProperties));
		properties.structSize = sizeof(vidEncVideoProperties);

		if (avifileinfo)
		{
			aviInfo info;
			video_body->getVideoInfo(&info);

			properties.width = info.width;
			properties.height = info.height;
			properties.parWidth = video_body->getPARWidth();
			properties.parHeight = video_body->getPARHeight();
			properties.frameCount = info.nb_frames;
			properties.fps1000 = info.fps1000;
		}

		configParameters.structSize = sizeof(vidEncConfigParameters);
		getMainWindowHandles(&configParameters.parent, &configParameters.parentNative);

		if (plugin->isConfigurable(plugin->encoderId))
			if (plugin->configure(plugin->encoderId, &configParameters, &properties))
			{
				int length = plugin->getOptions(plugin->encoderId, NULL, NULL, 0);
				char *pluginOptions = new char[length + 1];

				plugin->getOptions(plugin->encoderId, &encodeOptions, pluginOptions, length);
				pluginOptions[length] = 0;

				updateCompressionParameters(param, encodeOptions.encodeMode, encodeOptions.encodeModeParameter, pluginOptions, length);
			}
	}
	else
	{
		if (param->configure)
			param->configure(param);
	}
}

/*___________________________________________________________
	Set mode param and extra data for the currently selected codec
	The extradata is a free binary chunk (->memcpy of codec.specific part)
___________________________________________________________*/
void setVideoEncoderSettings(COMPRESSION_MODE mode, uint32_t param, uint32_t extraConf, uint8_t* extraData)
{
	COMPRES_PARAMS *zparam = &AllVideoCodec[currentCodecIndex];

	zparam->mode = mode;

	switch (mode)
	{
		case COMPRESS_SAME:
			aprintf ("Same as input\n");
			break;
		case COMPRESS_CBR:
			aprintf ("CBR : %lu kbps\n", param);
			zparam->bitrate = param;
			break;
		case COMPRESS_AQ:
			aprintf ("AQ : %lu q\n", param);
			zparam->qz = param;
			break;
		case COMPRESS_CQ:
			aprintf ("CQ : %lu q\n", param);
			zparam->qz = param;
			break;
		case COMPRESS_2PASS:
			aprintf ("2pass : %lu q\n", param);
			zparam->finalsize = param;
			break;
		case COMPRESS_2PASS_BITRATE:
			aprintf ("2passbitrate : %lu q\n", param);
			zparam->avg_bitrate = param;
			break;
		default:
			ADM_assert(0);
	}

	if (zparam->codec == CodecExternal)
	{
		ADM_vidEnc_plugin *plugin = getVideoEncoderPlugin(zparam->extra_param);
		vidEncOptions options;

		options.structSize = sizeof(vidEncOptions);
		options.encodeMode = getVideoEncodePluginMode(mode);
		options.encodeModeParameter = param;

		videoCodecSetConf(0, extraData);

		plugin->setOptions(plugin->encoderId, &options, (char*)zparam->extraSettings);
	}
	else if (extraConf && extraData && zparam->extraSettingsLen)
	{
		if (zparam->extraSettingsLen != extraConf)
			printf("*** ERROR : Extra data for codec config has a different size!!! *****\n");
		else
		{
			printf ("using %u bytes of codec specific data...\n", extraConf);
			memcpy (zparam->extraSettings, extraData, extraConf);
		}
	}
}

Encoder *getVideoEncoder(uint32_t w, uint32_t h, uint32_t globalHeaderFlag)
{
	Encoder *e = NULL;
	COMPRES_PARAMS *desc=getCodecParamFromTag(currentCodecType);
	switch (currentCodecType)
	{
	case CodecCopy:
		e = new EncoderCopy (NULL);
		break;
	default:
	case CodecYV12:
		e = new EncoderYV12 ();
		break;
	case CodecFF:
		e = new EncoderFFMPEG (FF_MPEG4, desc);
		break;
#if 0
	case CodecMjpeg:
		e = new EncoderMjpeg (&MjpegCodec);
		break;
#endif
	case CodecFFhuff:
		e = new EncoderFFMPEGFFHuff (desc);
		break;
	case CodecHuff:
		e = new EncoderFFMPEGHuff (desc);
		break;
	case CodecFLV1:
		e = new EncoderFFMPEGFLV1 (desc);
		break;
	case CodecFFV1:
		e = new EncoderFFMPEGFFV1 (desc);
		break;
	case CodecDV:
		e = new EncoderFFMPEGDV (desc);
		break;
	case CodecSnow:
		e = new EncodeFFMPEGSNow (desc);
		break;
	case CodecH263:
		if (!((w == 128) && (h == 96)) && !((w == 176) && (h == 144)))
		{
			GUI_Error_HIG (QT_TR_NOOP("Only QCIF and subQCIF are allowed for H.263"), NULL);
			return 0;
		}

		e = new EncoderFFMPEG (FF_H263, desc);
		break;
#ifdef USE_XVID_4
	case CodecXvid4:
		e = new EncoderXvid4 (desc);
		break;
#endif
	case CodecExternal:
		e = new externalEncoder(&AllVideoCodec[currentCodecIndex], globalHeaderFlag);
		break;
	}
	return e;
}

/**
	Select the codec and its configuration from a file
	(given as sole argument)

*/
uint8_t
loadVideoCodecConf (const char *name)
{
	if (currentCodecType == CodecExternal)
		videoCodecSetConf(strlen(name), (uint8_t*)name);
	else
	{
		FILE *fd = NULL;
		char str[4000];
		char str_extra[4000];
		char str_tmp[4000];
		uint32_t nb;
		uint32_t extraSize = 0;
		uint8_t *extra = NULL;

		fd = fopen (name, "rt");

		if (!fd)
		{
			printf ("Trouble reading codec conf\n");
			return 0;
		}

		fscanf (fd, "%s\n", str);
		// and video codec
		fscanf (fd, "Video codec : %s\n", str);
		videoCodecSelectByName (str);

		fgets (str, 200, fd);		// here we got the conf
		fscanf (fd, "Video extraConf size : %"LU"\n", &extraSize);

		if (extraSize)
		{
			uint8_t *ptr;
			fgets (str_tmp, 3999, fd);
			ptr = (uint8_t *) str_tmp;

			for (uint32_t k = 0; k < extraSize; k++)
			{
				str_extra[k] = mk_hex (*ptr, *(ptr + 1));
				ptr += 3;
			}

			extra = (uint8_t *) str_extra;
		}

		videoCodecSetConf(extraSize, extra);
		fclose (fd);
	}

	return 1;
}

uint8_t
loadVideoCodecConfString (const char *cmd)
{
#define MAX_STRING 4000
	char str[MAX_STRING * 3];
	char str_extra[MAX_STRING * 3];
	char str_tmp[MAX_STRING * 3];
	uint32_t nb;
	uint32_t extraSize = 0;
	uint8_t *extra = NULL;

	sscanf (cmd, "%d ", &extraSize);
	ADM_assert (extraSize < MAX_STRING);

	if (extraSize)
	{
		while (*cmd != ' ')
			cmd++;

		cmd++;	// skip the first ' '

		for (uint32_t k = 0; k < extraSize; k++)
		{
			str_extra[k] = mk_hex (*cmd, *(cmd + 1));
			cmd += 3;
		}

		extra = (uint8_t *)str_extra;
		videoCodecSetConf(extraSize, extra);
	}

	return 1;
}

uint8_t mk_hex(uint8_t a, uint8_t b)
{
	int a1 = a, b1 = b;

	if (a >= 'a')
	{
		a1 = a1 + 10;
		a1 = a1 - 'a';
	}
	else
		a1 = a1 - '0';

	if (b >= 'a')
	{
		b1 = b1 + 10;
		b1 = b1 - 'a';
	}
	else
		b1 = b1 - '0';

	return (a1 << 4) + b1;
}

uint8_t videoCodecGetConf (uint32_t * optSize, uint8_t ** data)
{
	*optSize = 0;
	*data = NULL;

	COMPRES_PARAMS *param = &AllVideoCodec[currentCodecIndex];

	if (param->extraSettingsLen)
	{
		*data = (uint8_t*)param->extraSettings;
		*optSize = param->extraSettingsLen;
	}

	printf ("Conf size: %d\n", *optSize);

	return 1;
}

void videoCodecSetConf(uint32_t extraLen, uint8_t *extraData)
{
	COMPRES_PARAMS *param = &AllVideoCodec[currentCodecIndex];

	if (param->codec == CodecExternal)
	{
		if (param->extraSettings)
			delete [] (char*)param->extraSettings;

		if (extraData)
		{
			param->extraSettingsLen = strlen((char*)extraData);
			param->extraSettings = ADM_strdup((char*)extraData);
		}
		else
		{
			param->extraSettings = NULL;
			param->extraSettingsLen = 0;
		}
	}
	else if (extraLen)
	{
		if (extraLen != param->extraSettingsLen)
		{
			printf("Codec:%s\n", param->descriptor);
			printf("Expected :%d got:%d\n", param->extraSettingsLen, extraLen);
			ADM_assert(0);
		}

		memcpy(param->extraSettings, extraData, param->extraSettingsLen);
	}
}

#ifdef USE_XVID_4
/**
    \fn     setIpod_Xvid4Preset(void)
    \brief  set XVID4 codec conf to the Ipod preset
*/
void setIpod_Xvid4Preset(void)
{
	memcpy(&xvid4Extra,&xvid4ExtraIPOD,sizeof(xvid4ExtraIPOD));
}
#endif

// EOF
