/***************************************************************************
                          ADM_edit.cpp  -  description
                             -------------------
    begin                : Thu Feb 28 2002
    copyright            : (C) 2002/2008 by mean
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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "A_functions.h"
#include "ADM_audioFilterInterface.h"
#include "audioEncoderApi.h"
#include "ADM_muxerProto.h"
#include "GUI_ui.h"

#include <fcntl.h>
#include <errno.h>

#if defined(__MINGW32__) || defined(ADM_BSD_FAMILY)
#include <sys/stat.h>
#endif

#include "fourcc.h"
#include "ADM_edit.hxx"
#include "ADM_edAudioTrackFromVideo.h"
#include "DIA_coreToolkit.h"
#include "prefs.h"

#include "ADM_debugID.h"
#define MODULE_NAME MODULE_EDITOR
#include "ADM_debug.h"

//#include "ADM_outputfmt.h"
#include "ADM_edPtsDts.h"
#include "ADM_vidMisc.h"
#include "ADM_confCouple.h"
#include "ADM_videoFilters.h"
#include "ADM_videoEncoderApi.h"
#include "ADM_videoFilterApi.h"

#if 0
/**
    \fn getVideoDecoderName
*/
const char          *ADM_Composer::getVideoDecoderName(void)
{
    if(!_segments.getNbRefVideos()) return "????";
     _VIDEOS *v=_segments.getRefVideo(0);
    if(!v) return "????";
    if(!v->decoder) return "????";
    return v->decoder->getDecoderName();
}
#endif
int ADM_Composer::setVideoCodec(const char *codec, CONFcouple *c)
{
	int idx = videoEncoder6_GetIndexFromName(codec);

	if (idx == -1)
	{
		ADM_error("No such encoder :%s\n", codec);
		return false;
	}

	// Select by index
	videoEncoder6_SetCurrentEncoder(idx);
	UI_setVideoCodec(idx);

	if (c)
	{
		bool r = videoEncoder6_SetConfiguration(c, true);
		delete c;

		return r;
	}

	return true;
}

int ADM_Composer::addVideoFilter(const char *filter, CONFcouple *c)
{
	uint32_t filterTag = ADM_vf_getTagFromInternalName(filter);

	printf("Adding Filter %s -> %"LU"... \n", filter, filterTag);

	bool r = ADM_vf_addFilterFromTag(filterTag, c, false);

	if (c)
	{
		delete c;
	}

	return r;
}


int ADM_Composer::saveAudio(const char *name)
{
	return A_audioSave(name); 
}


void ADM_Composer::clearFilters()
{
	ADM_vf_clearFilters();
}

int ADM_Composer::setAudioMixer(const char *s)
{
//    CHANNEL_CONF c = AudioMuxerStringToId(s);
//    return audioFilterSetMixer(c);
    return 0;
}

void ADM_Composer::resetAudioFilter()
{
//    audioFilterReset();
}

char *ADM_Composer::getVideoCodec(void)
{
	uint32_t fcc;
	aviInfo info;

	this->getVideoInfo(&info);
	fcc = info.fcc;

	return ADM_strdup(fourCC::tostring(fcc));
}

int ADM_Composer::appendFile(const char *name)
{
	return A_appendAvi(name);
}

uint32_t ADM_Composer::getAudioResample()
{
//	return audioFilterGetResample();
}

void ADM_Composer::setAudioResample(uint32_t newfq)
{
//	audioFilterSetResample(newfq);
}

int	ADM_Composer::openFile(const char *name)
{
	return A_openAvi(name);
}

int ADM_Composer::saveFile(const char *name)
{
	return A_Save(name);
}

bool ADM_Composer::setAudioCodec(const char *codec, int bitrate, CONFcouple *c)
{
#if 0
	bool r = true;

	// First search the codec by its name
	if (!audioCodecSetByName(codec))
	{
		r = false;
	}
	else
	{
		r = setAudioExtraConf(bitrate, c);
	}

	if (c)
		delete c;

	return r;
#endif
}

bool ADM_Composer::setContainer(const char *cont, CONFcouple *c)
{
	int idx = ADM_MuxerIndexFromName(cont);

	if (idx == -1)
	{
		ADM_error("Cannot find muxer for format=%s\n",cont);
		return false;
	}

	ADM_info("setting container as index %d\n",idx);
	UI_SetCurrentFormat(idx);
	idx = ADM_MuxerIndexFromName(cont);

	bool r = false;

	if(idx != -1)
	{
		r = ADM_mx_setExtraConf(idx, c);
	}

	if (c)
		delete c;

	return r;
}

bool ADM_Composer::setAudioFilterNormalise(ADM_GAINMode mode, uint32_t gain)
{
//	return audioFilterSetNormalize(mode, gain);
    return false;
}

bool ADM_Composer::getAudioFilterNormalise(ADM_GAINMode *mode, uint32_t *gain)
{
//	return audioFilterGetNormalize(mode, gain);
    return false;
}

FILMCONV ADM_Composer::getAudioFilterFrameRate(void)
{
	//return audioFilterGetFrameRate();
    return FILMCONV_NONE;
}

bool ADM_Composer::setAudioFilterFrameRate(FILMCONV conf)
{
	//return audioFilterSetFrameRate(conf);
    return false;
}

int ADM_Composer::saveImageBmp(const char *filename)
{
	return A_saveImg(filename);
}

int ADM_Composer::saveImageJpg(const char *filename)
{
	return A_saveJpg(filename);
}
#if 0
#endif
