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

#include "fourcc.h"
#include "ADM_edit.hxx"
#include "ADM_edAudioTrackFromVideo.h"
#include "ADM_edAudioTrackExternal.h"

#include "ADM_confCouple.h"

#include "ADM_filterCategory.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_videoFilters.h"
#include "ADM_videoFilterApi.h"

#include "ADM_audioFilterInterface.h"

#include "ADM_videoEncoderApi.h"
#include "audioEncoderApi.h"

#include "ADM_muxerProto.h"

#include "A_functions.h"
#include "GUI_ui.h"
#include "avi_vars.h"
#include "ADM_preview.h"

#include "gtkgui.h"
#include "prototype.h"

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
/**
 * \fn setVideoCodecProfile
 * \brief set a profile for the given video codec
 * @param codec
 * @param profile
 * @return 
 */
int         ADM_Composer::setVideoCodecProfile(const char *codec, const char *profile)
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

	if (profile)
	{
            ADM_info("Setting profile %s\n",profile);
            return videoEncoder6_SetProfile(profile);
	}

	return true;
}
/**
 * \fn changeVideoParam
 * \brief same as above but you can only change one param
 * @param codec
 * @param c
 * @return 
 */
int ADM_Composer::changeVideoParam(const char *codec, CONFcouple *c)
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
		bool r = videoEncoder6_SetConfiguration(c, false);
		delete c;

		return r;
	}

	return true; 
}
int ADM_Composer::addVideoFilter(const char *filter, CONFcouple *c)
{
    uint32_t filterTag = ADM_vf_getTagFromInternalName(filter);

    std::string name = filter;

    if (filterTag == VF_INVALID_FILTER)
    {
        if (false == ADM_vf_findCompatibleFilter(name, &c))
        {
            delete c;
            c = NULL;
            return 0;
        }
        // retry
        filterTag = ADM_vf_getTagFromInternalName(name.c_str());
        if (filterTag == VF_INVALID_FILTER)
        {
            ADM_warning("Cannot get valid tag for \"%s\".\n", name.c_str());
            delete c;
            c = NULL;
            return 0;
        }
    }
    ADM_info("Internal filter name \"%s\" matched to tag %" PRIu32"\n", name.c_str(), filterTag);

    bool r = (ADM_vf_addFilterFromTag(this, filterTag, c, false) != NULL);

    delete c;
    c = NULL;

    if (r)
        ADM_info("Filter \"%s\" added.\n", name.c_str());
    else
        ADM_warning("Cannot add filter \"%s\".\n", name.c_str());

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
    if(!_segments.getNbRefVideos())
        return A_openVideo(name);
    return A_appendVideo(name);
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
	return A_openVideo(name);
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

int ADM_Composer::saveImagePng(const char *filename)
{
    return A_savePng(filename);
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
    \fn setAudioShift
*/
bool ADM_Composer::setAudioShift(int dex, bool mode, int32_t shiftms)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    ed->audioEncodingConfig.audioFilterSetShift(mode,shiftms);
    return true;
}
/**
    \fn getAudioShift
*/
bool ADM_Composer::getAudioShift(int dex, bool *mode, int32_t *shiftms)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    ed->audioEncodingConfig.audioFilterGetShift(mode,shiftms);
    return true;
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
//#warning memleak on *c ?
    return true;
}

/**
    \fn setAudioFilterNormalise
*/
bool ADM_Composer::setAudioFilterNormalise(int dex, ADM_GAINMode mode, int32_t gain, int32_t maxlevel)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    return ed->audioEncodingConfig.audioFilterSetNormalize(mode,gain,maxlevel);
}
/**
    \fn getAudioFilterNormalise
*/
bool ADM_Composer::getAudioFilterNormalise(int dex, ADM_GAINMode *mode, int32_t *gain, int32_t *maxlevel)
{
    EditableAudioTrack *ed=getEditableAudioTrackAt(dex);
    if(!ed) return false;
    return ed->audioEncodingConfig.audioFilterGetNormalize(mode,gain,maxlevel);
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

/**
 * \fn      nextPicture
 * \brief   Preview or only decode the next picture
 */
bool ADM_Composer::nextPicture(ADMImage *image)
{
    if(!image)
        return GUI_NextFrame();
    return nextPicture(image,0);
}
/**
 * \fn      getCurrentFramePts
 * \brief   Get PTS from preview
 */
uint64_t ADM_Composer::getCurrentFramePts(void)
{
    return admPreview::getCurrentPts();
}
/**
 * \fn     setCurrentFramePts
 * \brief  Try to display the picture with the given PTS,
 *         else seek to the closest earlier keyframe
 */
bool ADM_Composer::setCurrentFramePts(uint64_t pts)
{
    if(admPreview::seekToTime(pts))
    {
        GUI_setCurrentFrameAndTime();
        UI_purge();
        return true;
    }
    // pts is not exact, can we seek to the previous keyframe?
    if(!getPKFramePTS(&pts)) // nope
        return false;
    return GUI_GoToKFrameTime(pts);
}
/**
 * \fn      getCurrentFrameFlags
 * \brief   Get flags from preview
 */
bool ADM_Composer::getCurrentFrameFlags(uint32_t *flags, uint32_t *quantizer)
{
    *flags = 0;
    if(!admPreview::getBuffer())
        return false;
    admPreview::getFrameFlags(flags, quantizer);
    return true;
}

_VIDEOS* ADM_Composer::getRefVideo(int videoIndex)
{
	return _segments.getRefVideo(videoIndex);
}

bool ADM_Composer::seekFrame(int count)
{
    if (!count)
        return admPreview::samePicture();

    bool r = true;
    admPreview::deferDisplay(true);
    if (count > 0)
    {
        for (int i = 0; i < count; i++)
        {
            if (!admPreview::nextPicture())
            {
                r = false;
                break;
            }
        }
    } else
    {
        for (int i = 0; i < -count; i++)
        {
            if (!admPreview::previousPicture())
            {
                r = false;
                break;
            }
        }
    }
    admPreview::deferDisplay(false);
    if (!admPreview::samePicture())
        return false;
    GUI_setCurrentFrameAndTime();
    UI_purge();
    return r;
}

bool ADM_Composer::seekKeyFrame(int count)
{
    uint64_t pts = admPreview::getCurrentPts();
    if(!pts && count < 0) // at the start of the video if any, nowhere to go
        return false;
    if(count < 1)
    {
        uint32_t flags,quant;
        admPreview::getFrameFlags(&flags,&quant);
        if(flags & AVI_KEY_FRAME)
        {
            if(!count) // already there, nothing to do
                return true;
        }else
        { // seek back -count keyframes starting with the closest earlier keyframe
            count--;
        }
    }

    int found = 0;
    if(count > 0)
    {
        for(int i = 0; i < count; i++)
        {
            uint64_t tmp = pts;
            if(!getNKFramePTS(&tmp))
                break;
            pts = tmp;
            found++;
        }
    }else
    {
        for(int i = 0; i < -count; i++)
        {
            uint64_t tmp = pts;
            if(!getPKFramePTS(&tmp))
                break;
            pts = tmp;
            found--;
        }
    }

    if(found && false == GUI_GoToKFrameTime(pts))
        return false;
    return found == count;
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
