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
	img.data = new uint8_t[2000 * 2000 * 3];
	img.dataLength = 2000 * 2000 * 3;

	if (!editor->getDirectImageForDebug(framenumber, &img))
	{
		ADM_error("Cannot get picture %d\n", framenumber);
		delete [] img.data;
		return false;
	}

	mixDump(img.data, img.dataLength);
	delete [] img.data;

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
		char field = 'F';

		if (flags & AVI_BOTTOM_FIELD) field = 'B';
		if (flags & AVI_TOP_FIELD) field = 'T';
		if (pts != ADM_NO_PTS && dts != ADM_NO_PTS) delta = (int64_t)pts - (int64_t)dts;
                printf("Frame  %05d",(int)framenumber);
                printf(" Flags %04x",(int)flags);
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
		ADM_warning("Cannot get PTS for frame %"LU"\n", frameNum);
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
		ADM_warning("Cannot get DTS for frame %"LU"\n", frameNum);
		return -1;
	}

	if (dts == ADM_NO_PTS)
	{
		return -1;
	}

	return (double)dts;
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

	if (!FileSel_SelectDir(QT_TR_NOOP("Select a directory"), me, 1023, NULL))
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
    \fn pyTestCrash
*/

int pyTestCrash(void)
{
	int *foobar = NULL;
	*foobar = 0; // CRASH!
	return true;
}
/**
    \fn pyTestAssert
*/

int pyTestAssert(void)
{
	ADM_assert(0);
	return true;
}
// EOF
