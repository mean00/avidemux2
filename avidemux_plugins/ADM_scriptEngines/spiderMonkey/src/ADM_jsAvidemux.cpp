#include "ADM_inttype.h"
#include "ADM_confCouple.h"
#include "ADM_jsAvidemux.h"
#include "ADM_jsUtils.h"
#include "SpiderMonkeyEngine.h"

using namespace std;

#define ENGINE(cx) ((SpiderMonkeyEngine*)JS_GetContextPrivate(cx))
#define EDITOR(cx) ENGINE(cx)->editor()

int   jsVideoCodec(const char *a,const char **b) {return 0;}

JSBool jsAdmvideoCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Codec
	*rval = BOOLEAN_TO_JSVAL(false);

	if(argc < 1)
		return JS_FALSE;

	if(!JSVAL_IS_STRING(argv[0]))
	{
		ENGINE(cx)->callEventHandlers(
			IScriptEngine::Information, NULL, -1, "Cannot set codec, first parameter is not a string");

		return JS_FALSE;
	}

	char *codec=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	CONFcouple *c;
	jsArgToConfCouple(argc-1,&c,argv+1);
	*rval = BOOLEAN_TO_JSVAL(EDITOR(cx)->setVideoCodec(codec, c));

	return JS_TRUE;
}// end Codec

int   jsVideoFilter(const char *a,const char **b) {return 0;}

JSBool jsAdmaddVideoFilter(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Codec
	uint32_t filterTag;

	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);

	if(argc == 0)
		return JS_FALSE;

	char *filterName=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	CONFcouple *c=NULL;

	if(argc)
		jsArgToConfCouple(argc-1,&c,argv+1);

	bool r = EDITOR(cx)->addVideoFilter(filterName, c);

	*rval=BOOLEAN_TO_JSVAL( r);
	return JS_TRUE;
}// end Codec

int jsAudioCodec(const char *a,const char **b) {return 0;}

JSBool jsAdmaudioCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);

	if (argc < 2)
		return JS_FALSE;

	if (!JSVAL_IS_STRING(argv[1]) || !JSVAL_IS_INT(argv[0]))
		return JS_FALSE;

	for (int i = 2; i < argc; i++)
		if (JSVAL_IS_STRING(argv[i]) == false)
			return JS_FALSE;

	// Get Codec...
    int dex=JSVAL_TO_INT(argv[0]);
	char *name = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));

	// Construct couples
	CONFcouple *c = NULL;

	if (argc > 2)
	{
		int nb = argc - 2;
		jsArgToConfCouple(nb, &c, argv + 2);
	}

	*rval = BOOLEAN_TO_JSVAL(EDITOR(cx)->setAudioCodec(dex,name, c));

	return JS_TRUE;
}

int   jsSetContainer(const char *a,const char **b) {return 0;}

JSBool jsAdmsetContainer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Codec
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);

	if (argc < 1)
	{
		ENGINE(cx)->callEventHandlers(
			IScriptEngine::Information, NULL, -1, "setContainer needs at least one arg");

		return JS_FALSE;
	}

	if (JSVAL_IS_STRING(argv[0]) == false)
	{
		ENGINE(cx)->callEventHandlers(
			IScriptEngine::Information, NULL, -1, "setContainer needs at string arg");

		return JS_FALSE;
	}

	char *str = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));

	ENGINE(cx)->callEventHandlers(
		IScriptEngine::Information, NULL, -1, (string("Selecting container: ") + string(str)).c_str());

	CONFcouple *c;
	jsArgToConfCouple(argc - 1, &c, argv + 1);

	*rval = BOOLEAN_TO_JSVAL(EDITOR(cx)->setContainer(str, c));

	return JS_TRUE;
}

void jsClearVideoFilters(JSContext *cx)
{
	EDITOR(cx)->clearFilters();
}

int jsAudioMixer(JSContext *cx, int dex,const char *s)
{
	return EDITOR(cx)->setAudioMixer(dex,s);
}

void jsAudioReset(JSContext *cx,int dex)
{
	EDITOR(cx)->resetAudioFilter(dex);
}

char *jsGetVideoCodec(JSContext *cx)
{
	return EDITOR(cx)->getVideoCodec();
}

int jsGetFps1000(JSContext *cx)
{
	aviInfo info;
	EDITOR(cx)->getVideoInfo(&info);

	return info.fps1000;
}

int jsGetWidth(JSContext *cx)
{
	aviInfo info;
	EDITOR(cx)->getVideoInfo(&info);

	return info.width;
}

int jsGetHeight(JSContext *cx)
{
	aviInfo info;
	EDITOR(cx)->getVideoInfo(&info);

	return info.height;
}

int jsAddSegment(JSContext *cx, int ref, double start, double duration)
{
	printf("1\n");
	if (EDITOR(cx)->addSegment(ref, (uint64_t)start, (uint64_t)duration))
	{
		printf("2\n");
		if (EDITOR(cx)->getNbSegment() == 1) // We just added our first seg...
		{
			printf("3\n");
			ENGINE(cx)->callEventHandlers(
				IScriptEngine::Information, NULL, -1, "First segment, refreshing screen");
			EDITOR(cx)->rewind();
			printf("4\n");
		}

		return 1;
	}

	return 0;
}

void jsClearSegments(JSContext *cx)
{
	EDITOR(cx)->clearSegment();
}

int jsSetPostProc(JSContext *cx, int a, int b, int c)
{
	return EDITOR(cx)->setPostProc(a, b, c);
}

int jsAppendVideo(JSContext *cx, const char *s)
{
	return EDITOR(cx)->appendFile(s);
}

double jsGetMarkerA(JSContext *cx)
{
	return (double)EDITOR(cx)->getMarkerAPts();
}

double jsGetMarkerB(JSContext *cx)
{
	return (double)EDITOR(cx)->getMarkerBPts();
}

void jsSetMarkerA(JSContext *cx, double a)
{
	EDITOR(cx)->setMarkerAPts((uint64_t)a);
}

void jsSetMarkerB(JSContext *cx, double b)
{
	EDITOR(cx)->setMarkerBPts((uint64_t)b);
}

uint32_t jsGetResample(JSContext *cx,int dex)
{
	return EDITOR(cx)->getAudioResample(dex);
}

void jsSetResample(JSContext *cx, int dex,uint32_t fq)
{
	return EDITOR(cx)->setAudioResample(dex,fq);
}

int jsLoadVideo(JSContext *cx, const char *s)
{
	return EDITOR(cx)->openFile(s);
}
