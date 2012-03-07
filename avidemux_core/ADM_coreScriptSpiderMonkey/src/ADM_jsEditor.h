#include "jsapi.h"

extern "C"
{
	int jsPrintTiming(JSContext *cx, int framenumber);
	int jsHexDumpFrame(JSContext *cx, int framenumber);
	int jsDumpSegments (JSContext *cx);
	int jsDumpRefVideos (JSContext *cx);
	JSBool dumpTiming(JSContext *cx);
	float scriptGetVideoDuration(JSContext *cx);
	double  scriptGetPts(JSContext *cx, int frameNum);
	double  scriptGetDts(JSContext *cx, int frameNum);
}
// EOF
