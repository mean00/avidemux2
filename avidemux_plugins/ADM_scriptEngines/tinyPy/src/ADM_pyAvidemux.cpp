/***************************************************************************
   \file ADM_pyAvidemux.cpp
    \brief binding between tinyPy and avidemux
    \author mean/gruntster 2011/2012
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_pyAvidemux.h"
#include "ADM_audiodef.h"
#include "ADM_vidMisc.h"
#include "fourcc.h"
#include "DIA_fileSel.h"
#include "DIA_coreToolkit.h"
#include "ADM_image.h"

/**
    \fn pyGetFps1000
*/
int pyGetFps1000(IEditor *editor)
{
	aviInfo info;
	editor->getVideoInfo(&info);

	return info.fps1000;
}
/**
    \fn pyGetWidth
*/

int pyGetWidth(IEditor *editor)
{
	aviInfo info;
	editor->getVideoInfo(&info);

	return info.width;
}
/**
    \fn pyGetHeight
*/

int pyGetHeight(IEditor *editor)
{
	aviInfo info;
	editor->getVideoInfo(&info);

	return info.height;
}
/**
    \fn pyHexDumpFrame
*/
int pyHexDumpFrame(IEditor *editor, int framenumber)
{
    ADMCompressedImage img;
    notStackAllocator buf(ADM_COMPRESSED_MAX_DATA_LENGTH);
    img.data = buf.data;
    img.dataLength = 0;

    if (!editor->getDirectImageForDebug(framenumber, &img))
    {
        ADM_error("Cannot get picture %d\n", framenumber);
        return 0;
    }

    mixDump(img.data, img.dataLength);

    return 1;
}

/**
 * \fn      pyNextFrame
 * \brief   decode next frame but do not update preview
 * @param   editor
 * @return  1 on success, else 0
 */
int pyNextFrame(IEditor *editor)
{
    aviInfo info;
    if(!editor->getVideoInfo(&info))
        return 0;
    ADMImageDefault img(info.width,info.height);
    return editor->nextPicture(&img);
}

/**
    \fn pyPrintTiming
*/

int pyPrintTiming(IEditor *editor, int framenumber)
{
    uint32_t flags;
    uint64_t pts, dts;

    if (editor->getVideoPtsDts(framenumber, &flags, &pts, &dts))
    {
        int64_t delta = 0;
        if (pts != ADM_NO_PTS && dts != ADM_NO_PTS)
            delta = (int64_t)pts - (int64_t)dts;
        const char *frameType;
        switch (flags & AVI_FRAME_TYPE_MASK)
        {
            case AVI_KEY_FRAME:
                frameType = "I";
                break;
            case AVI_P_FRAME:
                frameType = "P";
                break;
            case AVI_B_FRAME:
            case AVI_B_FRAME+AVI_NON_REF_FRAME:
                frameType = "B";
                break;
            default:
                frameType = "?";
                break;
        }
        const char *structureType;
        switch (flags & AVI_STRUCTURE_TYPE_MASK)
        {
            case AVI_FIELD_STRUCTURE+AVI_TOP_FIELD:
                structureType = "T";
                break;
            case AVI_FIELD_STRUCTURE+AVI_BOTTOM_FIELD:
                structureType = "B";
                break;
            case AVI_FRAME_STRUCTURE:
                structureType = "F";
                break;
            default:
                structureType = "?";
                break;
        }
        printf("Frame %05d",(int)framenumber);
        printf(" Flags %04x (%s/%s)",(int)flags,frameType,structureType);
        printf(" DTS %s",ADM_us2plain(dts));
        printf(" PTS %s\n",ADM_us2plain(pts));
    }

    return 0;
}

/**
    \fn pyPrintFrameInfo
*/
int pyPrintFrameInfo(IEditor *editor, int framenumber)
{
    uint32_t flags;
    uint64_t pts, dts;

    if (editor->getVideoPtsDts(framenumber, &flags, &pts, &dts))
    {
        int64_t delta = 0;
        if (pts != ADM_NO_PTS && dts != ADM_NO_PTS)
            delta = (int64_t)pts - (int64_t)dts;
        const char *frameType;
        switch ((flags & AVI_FRAME_TYPE_MASK) &~ AVI_NON_REF_FRAME)
        {
            case AVI_KEY_FRAME:
                frameType = "I";
                break;
            case AVI_P_FRAME:
                frameType = "P";
                break;
            case AVI_B_FRAME:
                frameType = "B";
                break;
            default:
                frameType = "?";
                break;
        }
        const char *structureType;
        switch (flags & AVI_STRUCTURE_TYPE_MASK)
        {
            case AVI_FIELD_STRUCTURE+AVI_TOP_FIELD:
                structureType = "T";
                break;
            case AVI_FIELD_STRUCTURE+AVI_BOTTOM_FIELD:
                structureType = "B";
                break;
            case AVI_FRAME_STRUCTURE:
                structureType = "F";
                break;
            default:
                structureType = "?";
                break;
        }

        uint64_t offset = 0;
        {
            _SEGMENT *seg = editor->getSegment(0);
            if(seg)
                offset = seg->_refStartTimeUs;
        }

        printf("Frame %05d",(int)framenumber);
        printf(" Flags %04x (%s/%s)",(int)flags,frameType,structureType);
        printf(" DTS %s",ADM_us2plain(dts));
        printf(" PTS %s",ADM_us2plain(pts));

        if(offset)
        {
            if(pts >= offset)
                printf(" / %s",ADM_us2plain(pts-offset));
            else
                printf(" /-%s",ADM_us2plain(offset-pts));
        }

        printf(" Size: %u\n",editor->getFrameSize(framenumber));
    }

    return 0;
}

/**
    \fn pyGetPts
*/

double pyGetPts(IEditor *editor, int frameNum)
{
	uint32_t flags;
	uint64_t pts, dts;

	if (!editor->getVideoPtsDts(frameNum, &flags, &pts, &dts))
	{
		ADM_warning("Cannot get PTS for frame %" PRIu32"\n", frameNum);
		return -1;
	}

	if (pts == ADM_NO_PTS)
	{
		return -1;
	}

	return (double)pts;
}
/**
    \fn pyGetDts
*/

double pyGetDts(IEditor *editor, int frameNum)
{
	uint32_t flags;
	uint64_t pts, dts;

	if (!editor->getVideoPtsDts(frameNum, &flags, &pts, &dts))
	{
		ADM_warning("Cannot get DTS for frame %" PRIu32"\n", frameNum);
		return -1;
	}

	if (dts == ADM_NO_PTS)
	{
		return -1;
	}

	return (double)dts;
}
/**
    \fn pyGetCurrentFrameFlags
*/
int pyGetCurrentFrameFlags(IEditor *editor)
{
    uint32_t flags,qz;
    if(false == editor->getCurrentFrameFlags(&flags,&qz))
        return -1;
    return flags;
}

/**
    \fn pyGetPrevKFramePts
*/
double pyGetPrevKFramePts(IEditor *editor, double time)
{
    uint64_t pts = ADM_NO_PTS;
    if(time < 0.)
        pts = editor->getCurrentFramePts();
    else
        pts = time;
    if(pts == ADM_NO_PTS)
        return -1;

    if(false == editor->getPKFramePTS(&pts))
        return -1;

    return (double)pts;
}

/**
    \fn pyGetNextKFramePts
*/
double pyGetNextKFramePts(IEditor *editor, double time)
{
    uint64_t pts = ADM_NO_PTS;
    if(time < 0.)
        pts = editor->getCurrentFramePts();
    else
        pts = time;
    if(pts == ADM_NO_PTS)
        return -1;

    if(false == editor->getNKFramePTS(&pts))
        return -1;

    return (double)pts;
}

/**
    \fn pySegmentGetRefIdx
*/
int pySegmentGetRefIdx(IEditor *editor, int segment)
{
    if(segment >= editor->getNbSegment())
        return -1;
    _SEGMENT* seg = editor->getSegment(segment);
    if(!seg)
        return -1;
    return seg->_reference;
}

/**
    \fn pySegmentGetTimeOffset
*/
double pySegmentGetTimeOffset(IEditor *editor, int segment)
{
    if(segment >= editor->getNbSegment())
        return -1;
    _SEGMENT *seg = editor->getSegment(segment);
    if(!seg)
        return -1;
    return seg->_refStartTimeUs;
}

/**
    \fn pySegmentGetDuration
*/
double pySegmentGetDuration(IEditor *editor, int segment)
{
    if(segment >= editor->getNbSegment())
        return -1;
    _SEGMENT *seg = editor->getSegment(segment);
    if(!seg)
        return -1;
    return seg->_durationUs;
}

/**
    \fn pyGetRefVideoDuration
*/
double pyGetRefVideoDuration(IEditor *editor, int idx)
{
    if(idx >= editor->getVideoCount())
        return -1;
    _VIDEOS *vid = editor->getRefVideo(idx);
    if(!vid)
        return -1;
    vidHeader *demuxer = vid->_aviheader;
    if(!demuxer)
        return -1;
    return demuxer->getVideoDuration();
}

/**
    \fn pyGetRefVideoName
*/
char *pyGetRefVideoName(IEditor *editor, int idx)
{
    if(idx >= editor->getVideoCount())
        return NULL;
    _VIDEOS *vid = editor->getRefVideo(idx);
    if(!vid)
        return NULL;
    vidHeader *demuxer = vid->_aviheader;
    if(!demuxer)
        return NULL;
    if(!demuxer->getMyName())
        return NULL;
    return ADM_strdup(demuxer->getMyName());
}

/**
    \fn pyFileSelWrite
*/

char *pyFileSelWrite(IEditor *editor, const char *title)
{
	char *me = NULL;

	GUI_FileSelWrite(title, &me);

	return me;
}

/**
    \fn pyFileSelRead
*/

char *pyFileSelRead(IEditor *editor, const char *title)
{
	char *me = NULL;

	GUI_FileSelRead(title, &me);

	return me;
}

#if defined(__APPLE__)
 #define MAX_LEN 1024
#else
 #define MAX_LEN 4096
#endif

/**
    \fn pyFileSelWriteEx
*/
char *pyFileSelWriteEx(IEditor *editor, const char *title, const char *ext)
{
    char me[MAX_LEN] = {0};
    const char *txt = QT_TRANSLATE_NOOP("tinypy","Save File");
    if(!FileSel_SelectWrite((title && strlen(title)) ? title : txt, me, MAX_LEN, NULL, ext))
        return NULL;

    return ADM_strdup(me);
}

/**
    \fn pyFileSelReadEx
*/
char *pyFileSelReadEx(IEditor *editor, const char *title, const char *ext)
{
    char me[MAX_LEN] = {0};
    const char *txt = QT_TRANSLATE_NOOP("tinypy","Open File");
    if(!FileSel_SelectRead((title && strlen(title)) ? title : txt, me, MAX_LEN, NULL, ext))
        return NULL;

    return ADM_strdup(me);
}

/**
    \fn pyDirSelect
*/

char *pyDirSelect(IEditor *editor, const char *title)
{
    char me[MAX_LEN] = {0};
    const char *txt = QT_TRANSLATE_NOOP("tinypy","Select Directory");
    if (!FileSel_SelectDir((title && strlen(title)) ? title : txt, me, MAX_LEN, NULL))
        return NULL;

    return ADM_strdup(me);
}
/**
    \fn pyDisplayError
*/

void pyDisplayError(IEditor *editor, const char *one, const char *two)
{
	GUI_Error_HIG(one, two);
}
/**
    \fn pyDisplayInfo
*/

void pyDisplayInfo(IEditor *editor, const char *one, const char *two)
{
	GUI_Info_HIG(ADM_LOG_INFO, one, two);
}

/**
 * \fn pyGetEnv
 * @param editor
 * @param 
 * @return 
 */
char *pyGetEnv(IEditor *editor,const char *key)
{
    const char *src="";
    if(editor)
    {
        if(key)
        {
            src=editor->getVar(key);
            if(!src) src="";
        }
    }
    return strdup(src);
}

/**
 * \fn pyGetContainerEx
 * \brief Get the default filename extension for the current muxer
 */
char *pyGetContainerEx(IEditor *editor)
{
    ADM_dynMuxer *container = editor->getCurrentMuxer();
    if(!container)
        return NULL;
    if(!container->defaultExtension)
        return NULL;
    return ADM_strdup(container->defaultExtension);
}
// EOF
