#include "ADM_pyAvidemux.h"
#include "ADM_audiodef.h"
#include "ADM_vidMisc.h"
#include "fourcc.h"
#include "DIA_fileSel.h"
#include "DIA_coreToolkit.h"

int pyChangeAudioStream(IEditor *editor, int track)
{
	editor->changeAudioStream(0, track);
}

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

static bool audioProlog(IEditor *editor, audioInfo **info)
{
	uint32_t  nbAudioTracks;
	audioInfo *infos = NULL;

	if (!editor->getAudioStreamsInfo(0, &nbAudioTracks, &infos))
	{
		ADM_warning("There is no audio track\n");
		return false;
	}

	int track = editor->getCurrentAudioStreamNumber(0);
	*info = infos + track;

	return true;
}

int pyGetAudioChannels(IEditor *editor)
{
	audioInfo *info = NULL;

	if (!audioProlog(editor, &info))
	{
		return 0;
	}

	return info->channels;
}

int pyGetAudioFrequency(IEditor *editor)
{
	audioInfo *info = NULL;

	if (!audioProlog(editor, &info))
	{
		return 0;
	}

	return info->frequency;

}

int pyGetAudioEncoding(IEditor *editor)
{
	audioInfo *info = NULL;

	if (!audioProlog(editor, &info))
	{
		return 0;
	}

	return info->encoding;
}

void pySetAudioFrequency(IEditor *editor, int fq)
{
	ADM_error("Cannot write audio frequency\n");
}

void pySetAudioEncoding(IEditor *editor, int enc)
{
	ADM_error("Cannot write audio encoding\n");
}

void pySetAudioChannels(IEditor *editor, int dq)
{
	ADM_error("Cannot write audio channel\n");
}

int32_t pyGetPal2Film(IEditor *editor)
{
	if (editor->getAudioFilterFrameRate() == FILMCONV_PAL2FILM)
	{
		return 1;
	}

	return 0;
}

int32_t pyGetFilm2Pal(IEditor *editor)
{
	if (editor->getAudioFilterFrameRate() == FILMCONV_FILM2PAL)
	{
		return 1;
	}

	return 0;
}

void pySetPal2Film(IEditor *editor, int32_t rate)
{
	if (rate)
	{
		editor->setAudioFilterFrameRate(FILMCONV_PAL2FILM);
	}
	else if (pyGetPal2Film(editor))
	{
		editor->setAudioFilterFrameRate(FILMCONV_NONE);
	}
}

void pySetFilm2Pal(IEditor *editor, int32_t rate)
{
	if (rate)
	{
		editor->setAudioFilterFrameRate(FILMCONV_FILM2PAL);
	}
	else if (pyGetFilm2Pal(editor))
	{
		editor->setAudioFilterFrameRate(FILMCONV_NONE);
	}
}

int pyGetNormalizeMode(IEditor *editor)
{
	ADM_GAINMode m;
	uint32_t gain;

	editor->getAudioFilterNormalise(&m, &gain);

	return m;
}

int pyGetNormalizeValue(IEditor *editor)
{
	ADM_GAINMode m;
	uint32_t gain;

	editor->getAudioFilterNormalise(&m, &gain);

	return (int)gain;
}

void pySetNormalizeMode(IEditor *editor, int mode)
{
	ADM_GAINMode m;
	uint32_t gain;

	editor->getAudioFilterNormalise(&m, &gain);
	m = (ADM_GAINMode)mode;
	editor->setAudioFilterNormalise(m, gain);
}

void pySetNormalizeValue(IEditor *editor, int value)
{
	ADM_GAINMode m;
	uint32_t gain;

	editor->getAudioFilterNormalise(&m, &gain);
	gain = (uint32_t)value;
	editor->setAudioFilterNormalise(m, gain);
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
