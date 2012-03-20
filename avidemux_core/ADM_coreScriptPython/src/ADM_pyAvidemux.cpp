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

#if 0
int pyChangeAudioStream(IEditor *editor, int track)
{
	editor->changeAudioStream(0, track);
}
#endif

int pyGetFps1000(IEditor *editor)
{
	aviInfo info;
	editor->getVideoInfo(&info);

	return info.fps1000;
}

int pyGetWidth(IEditor *editor)
{
	aviInfo info;
	editor->getVideoInfo(&info);

	return info.width;
}

int pyGetHeight(IEditor *editor)
{
	aviInfo info;
	editor->getVideoInfo(&info);

	return info.height;
}

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
	}

	return 0;
}

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

char *pyFileSelWrite(IEditor *editor, const char *title)
{
	char *me = NULL;

	GUI_FileSelWrite(title, &me);

	return me;
}

char *pyFileSelRead(IEditor *editor, const char *title)
{
	char *me = NULL;

	GUI_FileSelRead(title, &me);

	return me;
}

char *pyDirSelect(IEditor *editor, const char *title)
{
	char me[1024] = {0};

	if (!FileSel_SelectDir(QT_TR_NOOP("Select a directory"), me, 1023, NULL))
	{
		return NULL;
	}

	return ADM_strdup(me);
}

void pyDisplayError(IEditor *editor, const char *one, const char *two)
{
	GUI_Error_HIG(one, two);
}

void pyDisplayInfo(IEditor *editor, const char *one, const char *two)
{
	GUI_Info_HIG(ADM_LOG_INFO, one, two);
}

int pyTestCrash(void)
{
	int *foobar = NULL;
	*foobar = 0; // CRASH!
	return true;
}

int pyTestAssert(void)
{
	ADM_assert(0);
	return true;
}
