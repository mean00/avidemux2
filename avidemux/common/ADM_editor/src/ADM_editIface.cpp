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
#include "ADM_edAudioTrackExternal.h"

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
/**
     \fn dump
*/
bool PoolOfAudioTracks::dump(void)
{
    for(int i=0;i<size();i++)
    {
        ADM_edAudioTrack *track=at(i);
        ADM_info("Track %d, pool index=%d : \n",i,i);
        switch(track->getTrackType())
        {
            case ADM_EDAUDIO_FROM_VIDEO:
                        {
                        ADM_edAudioTrackFromVideo *vid=track->castToTrackFromVideo();
                        ADM_assert(vid);
                        ADM_info("\t track %d from video\n",vid->getMyTrackIndex());
                        }
                        break;
            case ADM_EDAUDIO_EXTERNAL:
                        {
                        ADM_edAudioTrackExternal *vid=track->castToExternal();
                        ADM_assert(vid);
                        ADM_info("\t from file %s\n",vid->getMyName().c_str());
                        }
                        break;
            default:
                    ADM_assert(0);
                    break;
        }
    }
    return true;
}
PoolOfAudioTracks::~PoolOfAudioTracks()
{
    for(int i=0;i<size();i++)
    {
        ADM_edAudioTrack *track=at(i);
        if(true==track->destroyable())
                delete track;
    }
    clear();
}
#if 0
 virtual ADM_edAudioTrackFromVideo *castToTrackFromVideo(void) 
                            {
                                    return NULL;
                            }
            virtual ADM_edAudioTrackExternal *castToExternal(void)
#endif
