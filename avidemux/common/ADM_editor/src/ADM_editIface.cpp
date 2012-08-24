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
#include "avi_vars.h"

#include <fcntl.h>
#include <errno.h>

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
#include "ADM_preview.h"
#include "ADM_edAudioTrackExternal.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_coreVideoFilterFunc.h"

extern uint8_t GUI_close(void);
extern bool GUI_GoToTime(uint64_t time);
extern void GUI_NextFrame(uint32_t frameCount);
extern void GUI_PrevFrame(uint32_t frameCount);
extern void GUI_NextKeyFrame(void);
extern void GUI_PreviousKeyFrame(void);
extern void GUI_PrevBlackFrame(void);
extern void GUI_NextBlackFrame(void);;

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

	printf("Adding Filter %s -> %"PRIu32"... \n", filter, filterTag);

	bool r = (ADM_vf_addFilterFromTag(this, filterTag, c, false) != NULL);

	if (c)
	{
		delete c;
	}

	return r;
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
/**
    \fn saveAudio
*/
int ADM_Composer::saveAudio(int dex,const char *name)
{
	return A_audioSave(name);
}


void ADM_Composer::clearFilters()
{
	ADM_vf_clearFilters();
}

int	ADM_Composer::openFile(const char *name)
{
	return A_openAvi(name);
}

int ADM_Composer::saveFile(const char *name)
{
	return A_Save(name);
}

ADM_dynMuxer* ADM_Composer::getCurrentMuxer()
{
	return ListOfMuxers[UI_GetCurrentFormat()];
}

ADM_videoEncoder6* ADM_Composer::getCurrentVideoEncoder()
{
	return ListOfEncoders[videoEncoder6_GetIndexFromName(videoEncoder6_GetCurrentEncoderName())];
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

int ADM_Composer::saveImageBmp(const char *filename)
{
	return A_saveImg(filename);
}

int ADM_Composer::saveImageJpg(const char *filename)
{
	return A_saveJpg(filename);
}
//----------------------------------AUDIO-------------------------
/**
    \fn setAudioMixer
*/
int ADM_Composer::setAudioMixer(int dex,const char *s)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    CHANNEL_CONF c = AudioMuxerStringToId(s);
    return ed->audioEncodingConfig.audioFilterSetMixer(c);
}
/**
    \fn resetAudioFilter
*/

void ADM_Composer::resetAudioFilter(int dex)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return ;
    ed->audioEncodingConfig.reset();

}
/**
    \fn setAudioDrc
*/

bool        ADM_Composer::setAudioDrc(int dex, bool mode)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    return ed->audioEncodingConfig.audioFilterSetDrcMode(mode);
}

/**
    \fn getAudioDrc
*/

bool        ADM_Composer::getAudioDrc(int dex)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    return ed->audioEncodingConfig.audioFilterGetDrcMode();
}


/**
    \fn getAudioResample
*/
uint32_t ADM_Composer::getAudioResample(int dex)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    return ed->audioEncodingConfig.audioFilterGetResample();
}
/**
    \fn setAudioResample
*/
void ADM_Composer::setAudioResample(int dex, uint32_t newfq)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return ;
    ed->audioEncodingConfig.audioFilterSetResample(newfq);
}
/**
    \fn setAudioCodec
*/
bool ADM_Composer::setAudioCodec(int dex,const char *codec, CONFcouple *c)
{
    if(!audioCodecSetByName(dex,codec))
    {
        ADM_warning("Cannot set codec %s, track %d\n",codec,dex);
        return false;
    }
    if(!setAudioExtraConf(dex,c))
    {
        ADM_warning("Cannot set configuration for codec %s, track %d\n",codec,dex);
        return false;
    }
#warning memleak on *c ?
    return true;
}

/**
    \fn setAudioFilterNormalise
*/
bool ADM_Composer::setAudioFilterNormalise(int dex,ADM_GAINMode mode, uint32_t gain)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    return ed->audioEncodingConfig.audioFilterSetNormalize(mode,gain);
}
/**
    \fn getAudioFilterNormalise
*/
bool ADM_Composer::getAudioFilterNormalise(int dex,ADM_GAINMode *mode, uint32_t *gain)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    return ed->audioEncodingConfig.audioFilterGetNormalize(mode,gain);
}
/**
    \fn getAudioFilterFrameRate
*/
FILMCONV ADM_Composer::getAudioFilterFrameRate(int dex)
{
	EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return FILMCONV_NONE;
    return ed->audioEncodingConfig.audioFilterGetFrameRate();
}
/**
    \fn setAudioFilterFrameRate
*/

bool ADM_Composer::setAudioFilterFrameRate(int dex,FILMCONV conf)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    return ed->audioEncodingConfig.audioFilterSetFrameRate(conf);
}
/**
    \fn clearAudioTracks
*/

bool        ADM_Composer::clearAudioTracks(void)
{
    // Remove all audio tracks
    activeAudioTracks.clear();
    return true;
}
/**
    \fn setAudioFilterFrameRate
*/

bool        ADM_Composer::addAudioTrack(int poolIndex)
{
    ADM_info("** Adding active track from pool **\n");
    activeAudioTracks.addTrack(poolIndex,audioTrackPool.at(poolIndex));
    ADM_info("Adding track %d\n",poolIndex);
    activeAudioTracks.dump();
    return true;
}
/**
    \fn addExternalAudioTrack
*/
bool    ADM_Composer::addExternalAudioTrack(const char *fileName)
{
    ADM_edAudioTrackExternal *ext=create_edAudioExternal(fileName);
    if(!ext)
    {
        ADM_warning("Error","Cannot use that file as audio track (%s)\n",fileName);
        return false;
    }else
    {
        ADM_info("Adding %s as external audio track %d\n",fileName,audioTrackPool.size());
        audioTrackPool.addInternalTrack(ext);
        audioTrackPool.dump();
        return true;
    }
}

int ADM_Composer::getVideoCount(void)
{
	return _segments.getNbRefVideos();
}

void ADM_Composer::closeFile(void)
{
	GUI_close();
}

_SEGMENT* ADM_Composer::getSegment(int i)
{
	return _segments.getSegment(i);
}

bool ADM_Composer::isFileOpen(void)
{
    return avifileinfo != NULL;
}

bool ADM_Composer::setCurrentFramePts(uint64_t pts)
{
    if (video_body->getPKFramePTS(&pts))
    {
        return GUI_GoToTime(pts);
    }
    else
    {
        return false;
    }
}

void ADM_Composer::getCurrentFrameFlags(uint32_t *flags, uint32_t *quantiser)
{
    admPreview::getFrameFlags(flags, quantiser);
}

_VIDEOS* ADM_Composer::getRefVideo(int videoIndex)
{
	return _segments.getRefVideo(videoIndex);
}

void ADM_Composer::seekFrame(int count)
{
	if (count >= 0)
	{
		for (int i = 0; i < count; i++)
		{
			GUI_NextFrame(1);
		}
	}
	else
	{
		for (int i = 0; i < -count; i++)
		{
			GUI_PrevFrame(1);
		}
	}
}

void ADM_Composer::seekKeyFrame(int count)
{
	if (count >= 0)
	{
		for (int i = 0; i < count; i++)
		{
			GUI_NextKeyFrame();
		}
	}
	else
	{
		for (int i = 0; i < -count; i++)
		{
			GUI_PreviousKeyFrame();
		}
	}
}

void ADM_Composer::seekBlackFrame(int count)
{
	int direction = (count > 0) ? 1 : -1;
	count = count * direction;

	for (int i = 0; i < count; i++)
	{
		if (direction == 1)
			GUI_NextBlackFrame();
		else
			GUI_PrevBlackFrame();
	}
}

void ADM_Composer::updateDefaultAudioTrack(void)
{
	EditableAudioTrack *ed = this->getDefaultEditableAudioTrack();

	if (ed)
	{
		UI_setAudioCodec(ed->encoderIndex);
	}
}
