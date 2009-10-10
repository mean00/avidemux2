//
// C++ Interface: Spider Monkey interface
//
// Description: 
//
//
// Author: Anish Mistry
//      Some modification by mean
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ADM_default.h"

#include "ADM_JSAvidemuxVideo.h"
#include "ADM_JSGlobal.h"


#include "ADM_commonUI/GUI_ui.h"
#include "avi_vars.h"
#include "gui_action.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_editor/ADM_outputfmt.h"
#include "ADM_commonUI/GUI_ui.h"
#include "ADM_script/ADM_container.h"

extern VF_FILTERS filterGetTagFromName(const char *inname);
extern uint8_t A_ListAllBlackFrames( char *file );
extern uint8_t loadVideoCodecConfString( const char *name);
extern uint8_t ADM_saveRaw (const char *name);
extern int A_saveJpg (char *name);
extern uint8_t loadVideoCodecConf( const char *name);
extern void filterCleanUp( void );

JSPropertySpec ADM_JSAvidemuxVideo::avidemuxvideo_properties[] = 
{ 
        { "process", videoprocess_prop, JSPROP_ENUMERATE },        // process video when saving
	{ 0 }
};

JSFunctionSpec ADM_JSAvidemuxVideo::avidemuxvideo_methods[] = 
{
	{ "clear", Clear, 0, 0, 0 },	// clear
	{ "add", Add, 3, 0, 0 },	// add
        { "clearFilters", ClearFilters, 0, 0, 0 }, // Delete all filters
	{ "addFilter", AddFilter, 10, 0, 0 },	// Add filter to filter chain
	{ "codec", Codec, 3, 0, 0 },	// Set the video codec
	{ "codecPlugin", codecPlugin, 4, 0, 0 },	// Set the video codec plugin
	{ "codecConf", CodecConf, 1, 0, 0 },	// load video codec config
	{ "save", Save, 1, 0, 0 },	// save video portion of the stream
	{ "saveJpeg", SaveJPEG, 1, 0, 0 },	// save the current frame as a JPEG
	{ "listBlackFrames", ListBlackFrames, 1, 0, 0 },	// output a list of the black frame to a file
	{ "setPostProc", PostProcess, 3, 0, 0 },	// Postprocess
        { "setFps1000", SetFps1000, 1, 0, 0 },        // Postprocess
        { "getFps1000", GetFps1000, 0, 0, 0 },        // Postprocess
        { "getNbFrames", GetNbFrames, 0, 0, 0 },        // Postprocess
        { "getWidth", GetWidth, 0, 0, 0 },        // Postprocess
        { "getHeight", GetHeight, 0, 0, 0 },        // Postprocess
        { "getFCC", GetFCC, 0, 0, 0 },        // Postprocess
        { "isVopPacked", isVopPacked, 0, 0, 0 },        // Postprocess
        { "hasQpel", hasQpel, 0, 0, 0 },        // Postprocess
        { "hasGmc", hasGmc, 0, 0, 0 },        // Postprocess
        { "frameSize", getFrameSize, 1, 0, 0 },        // FrameSize
        { "frameType", getFrameType, 1, 0, 0 },        // Postprocess
	{ 0 }
};

JSClass ADM_JSAvidemuxVideo::m_classAvidemuxVideo = 
{
	"AvidemuxVideo", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	ADM_JSAvidemuxVideo::JSGetProperty, ADM_JSAvidemuxVideo::JSSetProperty,
	JS_EnumerateStub, JS_ResolveStub, 
	JS_ConvertStub, ADM_JSAvidemuxVideo::JSDestructor
};

ADM_JSAvidemuxVideo::~ADM_JSAvidemuxVideo(void)
{
	if(m_pObject != NULL)
		delete m_pObject;
	m_pObject = NULL;
}

void ADM_JSAvidemuxVideo::setObject(ADM_AvidemuxVideo *pObject)
{
	m_pObject = pObject; 
}
	
ADM_AvidemuxVideo *ADM_JSAvidemuxVideo::getObject()
{
	return m_pObject; 
}

JSObject *ADM_JSAvidemuxVideo::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
        JSObject *newObj = JS_InitClass(cx, obj, proto, &m_classAvidemuxVideo, 
                                        ADM_JSAvidemuxVideo::JSConstructor, 0,
                                        ADM_JSAvidemuxVideo::avidemuxvideo_properties, ADM_JSAvidemuxVideo::avidemuxvideo_methods,
                                        NULL, NULL);
	return newObj;
}

JSBool ADM_JSAvidemuxVideo::JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								 jsval *argv, jsval *rval)
{
        if(argc != 0)
                return JS_FALSE;
        ADM_JSAvidemuxVideo *p = new ADM_JSAvidemuxVideo();
        ADM_AvidemuxVideo *pObject = new ADM_AvidemuxVideo();
        p->setObject(pObject);
        if ( ! JS_SetPrivate(cx, obj, p) )
                return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
}

void ADM_JSAvidemuxVideo::JSDestructor(JSContext *cx, JSObject *obj)
{
        ADM_JSAvidemuxVideo *p = (ADM_JSAvidemuxVideo *)JS_GetPrivate(cx, obj);
        if(p != NULL)
                delete p;
        p = NULL;
}

JSBool ADM_JSAvidemuxVideo::JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
        if (JSVAL_IS_INT(id)) 
        {
                ADM_JSAvidemuxVideo *priv = (ADM_JSAvidemuxVideo *) JS_GetPrivate(cx, obj);
                switch(JSVAL_TO_INT(id))
                {
                        case videoprocess_prop:
                                *vp = BOOLEAN_TO_JSVAL(priv->getObject()->m_bVideoProcess);
                                break;
                }
        }
        return JS_TRUE;
}

JSBool ADM_JSAvidemuxVideo::JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
        if (JSVAL_IS_INT(id)) 
        {
                ADM_JSAvidemuxVideo *priv = (ADM_JSAvidemuxVideo *) JS_GetPrivate(cx, obj);
                switch(JSVAL_TO_INT(id))
                {
                        case videoprocess_prop:
                                if(JSVAL_IS_BOOLEAN(*vp) == false)
                                        break;
                                priv->getObject()->m_bVideoProcess = JSVAL_TO_BOOLEAN(*vp);
                                UI_setVProcessToggleStatus(priv->getObject()->m_bVideoProcess);
                                break;
                }
        }
        return JS_TRUE;
}

JSBool ADM_JSAvidemuxVideo::Clear(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin Clear
        ADM_JSAvidemuxVideo *p = (ADM_JSAvidemuxVideo *)JS_GetPrivate(cx, obj);
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc != 0)
                return JS_FALSE;
        printf("Clearing Video... \n");
        enterLock();
//        *rval = BOOLEAN_TO_JSVAL(video_body->deleteAllSegments());
        leaveLock();
        return JS_TRUE;
}// end Clear

JSBool ADM_JSAvidemuxVideo::Add(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin Add
        ADM_JSAvidemuxVideo *p = (ADM_JSAvidemuxVideo *)JS_GetPrivate(cx, obj);
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc != 3)
                return JS_FALSE;
        if(JSVAL_IS_INT(argv[0]) == false || JSVAL_IS_INT(argv[1]) == false  || JSVAL_IS_INT(argv[2]) == false)
                return JS_FALSE;
        printf("Adding Video... \n");
        enterLock();
//        *rval = BOOLEAN_TO_JSVAL(video_body->addSegment(JSVAL_TO_INT(argv[0]),JSVAL_TO_INT(argv[1]),JSVAL_TO_INT(argv[2])));
        leaveLock();
        return JS_TRUE;
}// end Add


JSBool ADM_JSAvidemuxVideo::ClearFilters(JSContext *cx, JSObject *obj, uintN argc,
                                       jsval *argv, jsval *rval)
{// begin Clear
//	filterCleanUp();
        return JS_TRUE;
}// end Clear


JSBool ADM_JSAvidemuxVideo::AddFilter(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin AddFilter
#if 0
        VF_FILTERS filter;

        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc == 0)
                return JS_FALSE;

        filter = filterGetTagFromName(JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));
        printf("Adding Filter \"%d\"... \n",filter);

        Arg args[argc];
        char *v;
        for(int i=0;i<argc;i++) 
        {
                args[i].type=APM_STRING;
                if(JSVAL_IS_STRING(argv[i]) == false)
                {
                        return JS_FALSE;
                }
                v=ADM_strdup(JS_GetStringBytes(JSVAL_TO_STRING(argv[i])));
                args[i].arg.string=v;
        }
        enterLock();
        *rval= BOOLEAN_TO_JSVAL(filterAddScript(filter,argc,args));
        leaveLock();
        
        for(int i=0;i<argc;i++) 
        {
            ADM_dealloc(args[i].arg.string);
        }
#endif        
        return JS_TRUE;
}// end AddFilter

JSBool ADM_JSAvidemuxVideo::Codec(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin Codec
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc > 3)
                return JS_FALSE;
        printf("Codec ... \n");
        if(JSVAL_IS_STRING(argv[0]) == false || JSVAL_IS_STRING(argv[1]) == false  || JSVAL_IS_STRING(argv[2]) == false)
                return JS_FALSE;
        
                printf("[codec]%s\n",JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));
                printf("[conf ]%s\n",JS_GetStringBytes(JSVAL_TO_STRING(argv[1])));
                printf("[xtra ]%s\n",JS_GetStringBytes(JSVAL_TO_STRING(argv[2])));
                
                char *codec,*conf,*codecConfString;
                codec = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
                conf = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
                codecConfString = JS_GetStringBytes(JSVAL_TO_STRING(argv[2]));
                enterLock();
#if 0
                if(!videoCodecSelectByName(codec))
                        *rval = BOOLEAN_TO_JSVAL(false);
                else
                {// begin conf
                        // now do the conf
                        // format CBR=bitrate in kbits
                        //	  CQ=Q
                        //	  2 Pass=size
                        // We have to replace
                        if(!videoCodecConfigure(conf,0,NULL))
                                *rval = BOOLEAN_TO_JSVAL(false);
                        else
                        {
                                *rval = BOOLEAN_TO_JSVAL(true);
                                if(!loadVideoCodecConfString(codecConfString))
                                        *rval = BOOLEAN_TO_JSVAL(false);
                                else
                                        *rval = BOOLEAN_TO_JSVAL(true);
                        }

                }// end conf
#endif
                leaveLock();

        return JS_TRUE;
}// end Codec

JSBool ADM_JSAvidemuxVideo::codecPlugin(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	*rval = BOOLEAN_TO_JSVAL(false);
#if 0
	if (argc != 4)
		return JS_FALSE;

	printf("Codec Plugin ... \n");

	if (!JSVAL_IS_STRING(argv[0]) || !JSVAL_IS_STRING(argv[1]) || !JSVAL_IS_STRING(argv[2]) || !JSVAL_IS_STRING(argv[3]))
		return JS_FALSE;

	char *guid = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	char *desc = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
	char *conf = JS_GetStringBytes(JSVAL_TO_STRING(argv[2]));
	char *data = JS_GetStringBytes(JSVAL_TO_STRING(argv[3]));

	printf("[guid] %s\n", guid);
	printf("[desc] %s\n", desc);
	printf("[conf] %s\n", conf);
	printf("[data] %s\n", data);

	enterLock();

	if (!videoCodecPluginSelectByGuid(guid))
		*rval = BOOLEAN_TO_JSVAL(false);
	else
		*rval = BOOLEAN_TO_JSVAL(videoCodecConfigure(conf, 0, (uint8_t*)data));

	leaveLock();
#endif
	return JS_TRUE;
}

JSBool ADM_JSAvidemuxVideo::CodecConf(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{
	*rval = BOOLEAN_TO_JSVAL(false);
#if 0
	if (argc != 1)
		return JS_FALSE;

	if (!JSVAL_IS_STRING(argv[0]))
		return JS_FALSE;

	char *pTempStr = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));

	printf("Codec Conf Video \"%s\"\n", pTempStr);

	enterLock();
	*rval = INT_TO_JSVAL(loadVideoCodecConf(pTempStr));
	leaveLock();
#endif
	return JS_TRUE;
}

JSBool ADM_JSAvidemuxVideo::Save(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin Save
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
#if 0
        if(argc != 1)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false)
                return JS_FALSE;
        char *pTempStr = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        printf("Saving Video \"%s\"\n",pTempStr);
        enterLock();
        *rval = INT_TO_JSVAL(ADM_saveRaw(pTempStr));
        leaveLock();
#endif
        return JS_TRUE;
}// end Save

JSBool ADM_JSAvidemuxVideo::SaveJPEG(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin SaveJPG
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
#if 0
        if(argc != 1)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false)
                return JS_FALSE;
        char *pTempStr = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        printf("Saving JPEG \"%s\"\n",pTempStr);
        enterLock();
        *rval = INT_TO_JSVAL(A_saveJpg(pTempStr));
        leaveLock();
#endif
        return JS_TRUE;
}// end SaveJPG

JSBool ADM_JSAvidemuxVideo::ListBlackFrames(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin ListBlackFrames
        
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
#if 0
        if(argc != 1)
          return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false)
          return JS_FALSE;
        
        enterLock();
        A_ListAllBlackFrames(JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));
        leaveLock();
        *rval = BOOLEAN_TO_JSVAL(true);
#endif
        return JS_TRUE;
}// end ListBlackFrames

JSBool ADM_JSAvidemuxVideo::PostProcess(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc != 3)
                return JS_FALSE;
        if(JSVAL_IS_INT(argv[0]) == false || JSVAL_IS_INT(argv[1]) == false || JSVAL_IS_INT(argv[2]) == false)
                return JS_FALSE;
        
        enterLock();
        int rtn =video_body->setPostProc(
            JSVAL_TO_INT(argv[0]),JSVAL_TO_INT(argv[1]),JSVAL_TO_INT(argv[2]));
        leaveLock();
        *rval = BOOLEAN_TO_JSVAL(rtn);
        return JS_TRUE;
}// end PostProcess

JSBool ADM_JSAvidemuxVideo::GetFps1000(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
aviInfo info;
        if(argc != 0)
          return JS_FALSE;
        
        enterLock();
        video_body->getVideoInfo(&info);
        leaveLock();
        
        *rval = INT_TO_JSVAL(info.fps1000);
        return JS_TRUE;
}// end PostProcess
JSBool ADM_JSAvidemuxVideo::GetNbFrames(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
aviInfo info;
        if(argc != 0)
          return JS_FALSE;
        
        enterLock();
        video_body->getVideoInfo(&info);
        leaveLock();
        
        *rval = INT_TO_JSVAL(info.nb_frames);
        return JS_TRUE;
}// end PostProcess

JSBool ADM_JSAvidemuxVideo::SetFps1000(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
int fps;
aviInfo info;

        if(argc != 1)
          return JS_FALSE;
        if(JSVAL_IS_INT(argv[0]) == false)
          return JS_FALSE;

        enterLock();
        video_body->getVideoInfo(&info);
        video_body->getVideoInfo (avifileinfo);


        // default return value
        fps=JSVAL_TO_INT(argv[0]);
        if(fps>100000 || fps<2000)
        {      
                printf("Fps too low\n");
                leaveLock();
                return JS_FALSE;
        }       
 	
       info.fps1000=fps;
        video_body->updateVideoInfo(&info);
        video_body->getVideoInfo (avifileinfo);
        
	leaveLock();
        return JS_TRUE;
}// end PostProcess


JSBool ADM_JSAvidemuxVideo::GetWidth(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
aviInfo info;
        if(argc != 0)
          return JS_FALSE;


        enterLock();
        video_body->getVideoInfo(&info);
        leaveLock();
        
        *rval = INT_TO_JSVAL(info.width);
        return JS_TRUE;
}// end PostProcess
JSBool ADM_JSAvidemuxVideo::GetHeight(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
aviInfo info;
        if(argc != 0)
          return JS_FALSE;

        enterLock();
        video_body->getVideoInfo(&info);
        leaveLock();
        
        *rval = INT_TO_JSVAL(info.height);
        return JS_TRUE;
}// end PostProcess
JSBool ADM_JSAvidemuxVideo::GetFCC(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
aviInfo info;

        if(argc != 0)
          return JS_FALSE;

        enterLock();
        video_body->getVideoInfo(&info);
        leaveLock();
        
        *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fourCC::tostring(info.fcc)));
        return JS_TRUE;
}// end PostProcess

JSBool ADM_JSAvidemuxVideo::isVopPacked(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
int32_t info;
        if(argc != 0)
          return JS_FALSE;

      enterLock();
       info=video_body->getSpecificMpeg4Info();
       leaveLock();
         
        // default return value
        *rval=JS_FALSE;
        if(info & ADM_VOP_ON) *rval=JS_TRUE;
        return JS_TRUE;
}// end PostProcess
JSBool ADM_JSAvidemuxVideo::hasGmc(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
uint32_t info;
        if(argc != 0)
          return JS_FALSE;

       enterLock();
       info=video_body->getSpecificMpeg4Info();
       leaveLock(); 
        
        // default return value
        *rval=JS_FALSE;
        if(info & ADM_GMC_ON) *rval=JS_TRUE;
        return JS_TRUE;
}// end PostProcess
JSBool ADM_JSAvidemuxVideo::hasQpel(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
uint32_t info;
        if(argc != 0)
          return JS_FALSE;
        
        enterLock();
        info=video_body->getSpecificMpeg4Info();
        leaveLock(); 
        
        *rval=JS_FALSE;
        if(info & ADM_QPEL_ON) *rval=JS_TRUE;
        return JS_TRUE;
}// end PostProcess


JSBool ADM_JSAvidemuxVideo::getFrameSize(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
uint32_t info;
uint32_t frame;
uint32_t sz;
        if(argc != 1)
          return JS_FALSE;
        
        enterLock();
        frame=JSVAL_TO_INT(argv[0]);
        if(!video_body->getFrameSize(frame,&sz)) return JS_FALSE;
        leaveLock(); 
        
        *rval=INT_TO_JSVAL(sz);
        return JS_TRUE;
}// end PostProcess
JSBool ADM_JSAvidemuxVideo::getFrameType(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
uint32_t info;
uint32_t frame;
uint32_t sz;
        if(argc != 1)
          return JS_FALSE;
        
        enterLock();
        frame=JSVAL_TO_INT(argv[0]);
        if(!video_body->getFlags(frame,&sz)) return JS_FALSE;
        leaveLock(); 
        
        *rval=INT_TO_JSVAL(sz);
        return JS_TRUE;
}// end PostProcess
/* EOF */
