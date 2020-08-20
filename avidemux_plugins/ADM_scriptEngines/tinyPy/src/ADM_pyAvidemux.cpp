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

bool pyHexDumpFrame(IEditor *editor, int framenumber)
{
    ADMCompressedImage img;
    notStackAllocator buf(ADM_COMPRESSED_MAX_DATA_LENGTH);
    img.data = buf.data;
    img.dataLength = 0;

    if (!editor->getDirectImageForDebug(framenumber, &img))
    {
        ADM_error("Cannot get picture %d\n", framenumber);
        return false;
    }

    mixDump(img.data, img.dataLength);

    return true;
}

/**
 * 
 * @param editor
 * @param framenumber
 * @return 
 */
bool pyNextFrame(IEditor *editor)
{
    aviInfo info;
    if(!editor->getVideoInfo(&info))
        return false;
    
    ADMImageDefault img(info.width,info.height);
    if(!editor->nextPicture(&img,false))
       return false;
    return true;
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
    \fn pyGetPrevKFramePts
*/
double pyGetPrevKFramePts(IEditor *editor)
{
    uint64_t pts = editor->getCurrentFramePts();
    if(pts == ADM_NO_PTS)
        return -1;

    if(false == editor->getPKFramePTS(&pts))
        return -1;

    return (double)pts;
}

/**
    \fn pyGetNextKFramePts
*/
double pyGetNextKFramePts(IEditor *editor)
{
    uint64_t pts = editor->getCurrentFramePts();
    if(pts == ADM_NO_PTS)
        return -1;

    if(false == editor->getNKFramePTS(&pts))
        return -1;

    return (double)pts;
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
/**
    \fn pyDirSelect
*/

char *pyDirSelect(IEditor *editor, const char *title)
{
	char me[1024] = {0};

	if (!FileSel_SelectDir((title && strlen(title)) ? title : QT_TR_NOOP("Select a directory"), me, 1023, NULL))
	{
		return NULL;
	}

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

// EOF
