#include <sstream>
#include "ADM_inttype.h"
#include "ADM_vidMisc.h"
#include "SpiderMonkeyEngine.h"
#include "ADM_jsEditor.h"

using namespace std;

void mixDump(uint8_t *ptr, uint32_t len);

/**
\fn jsPrintTiming
*/
int jsPrintTiming(JSContext *cx, int framenumber)
{
	uint32_t flags;
	uint64_t pts, dts;

	SpiderMonkeyEngine *engine = (SpiderMonkeyEngine*)JS_GetContextPrivate(cx);
	IEditor *videoBody = engine->editor();

	if (videoBody->getVideoPtsDts(framenumber, &flags, &pts, &dts))
	{
		int64_t delta = 0;
		char field = 'F';

		if (flags & AVI_BOTTOM_FIELD) field = 'B';
		if (flags & AVI_TOP_FIELD) field = 'T';
		if (pts != ADM_NO_PTS && dts != ADM_NO_PTS) delta = (int64_t)pts - (int64_t)dts;

		stringstream stream;

		stream << "Frame " << framenumber << " PIC: " << field << " Flags " << flags << " pts=" << pts <<
			" pts=" << ADM_us2plain(pts) << " dts=" << dts << " delta=" << delta / 1000LL << " ms";

		engine->callEventHandlers(IScriptEngine::Information, NULL, -1, stream.str().c_str());
	}
	else
	{
		stringstream stream;

		stream << "Cannot get info for frame " << framenumber;

		engine->callEventHandlers(IScriptEngine::Information, NULL, -1, stream.str().c_str());
	}

	return 0;
}

/**
\fn jsHexDumpFrame
*/
int jsHexDumpFrame(JSContext *cx, int framenumber)
{
	ADMCompressedImage img;
	SpiderMonkeyEngine *engine = (SpiderMonkeyEngine*)JS_GetContextPrivate(cx);
	IEditor *videoBody = engine->editor();

	img.data = new uint8_t[2000 * 2000 * 3];
	img.dataLength = 2000 * 2000 * 3;

	if (!videoBody->getDirectImageForDebug(framenumber, &img))
	{
		stringstream stream;

		stream << "Cannot get picture " << framenumber;
		engine->callEventHandlers(IScriptEngine::Information, NULL, -1, stream.str().c_str());
		delete [] img.data;

		return false;
	}

	mixDump(img.data,img.dataLength);
	delete [] img.data;

	return true;
}

/**
\fn    jsDumpSegments
\brief dump segment, video & all
*/
int jsDumpSegments (JSContext *cx)
{// begin PostProcess
	IEditor *videoBody = ((SpiderMonkeyEngine*)JS_GetContextPrivate(cx))->editor();

	videoBody->dumpSegments();

	return 0;
}// end PostProcess

/**
\fn jsDumpRefVideos
*/
int jsDumpRefVideos (JSContext *cx)
{
	IEditor *videoBody = ((SpiderMonkeyEngine*)JS_GetContextPrivate(cx))->editor();

	videoBody->dumpRefVideos();

	return 0;
}

/**
\fn dumpTiming
\brief dump segment, video & all
*/
JSBool dumpTiming(JSContext *cx)
{// begin PostProcess
	IEditor *videoBody = ((SpiderMonkeyEngine*)JS_GetContextPrivate(cx))->editor();

	videoBody->dumpTiming();

	return 0;
}// end PostProcess

/**
\fn scriptGetVideoDuration
*/
float scriptGetVideoDuration(JSContext *cx)
{
	IEditor *videoBody = ((SpiderMonkeyEngine*)JS_GetContextPrivate(cx))->editor();
	uint64_t d = videoBody->getVideoDuration();

	return (float)d;
}

/**
\fn scriptGetPts
*/
double  scriptGetPts(JSContext *cx, int frameNum)
{
	uint32_t flags;
	uint64_t pts, dts;
	IEditor *videoBody = ((SpiderMonkeyEngine*)JS_GetContextPrivate(cx))->editor();

	if(!videoBody->getVideoPtsDts(frameNum, &flags, &pts, &dts))
	{
		ADM_warning("Cannot get PTS for frame %"LU"\n", frameNum);
		return -1;
	}

	if(pts == ADM_NO_PTS) return -1;

	return (double)pts;
}

/**
\fn scriptGetDts
*/
double  scriptGetDts(JSContext *cx, int frameNum)
{
	uint32_t flags;
	uint64_t pts,dts;
	IEditor *videoBody = ((SpiderMonkeyEngine*)JS_GetContextPrivate(cx))->editor();

	if(!videoBody->getVideoPtsDts(frameNum, &flags, &pts, &dts))
	{
		ADM_warning("Cannot get DTS for frame %"LU"\n", frameNum);
		return -1;
	}

	if(dts == ADM_NO_PTS) return -1;

	return (double)dts;
}
