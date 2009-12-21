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


#include "ADM_editor/ADM_outputfmt.h"
#include "ADM_commonUI/GUI_ui.h"
#include "ADM_script/ADM_container.h"
#include "ADM_videoEncoderApi.h"
#include "ADM_videoFilterApi.h"
#include "ADM_videoFilters.h"
#include "ADM_confCouple.h"
extern uint8_t A_ListAllBlackFrames( char *file );
extern uint8_t ADM_saveRaw (const char *name);
extern int A_saveJpg (char *name);
extern void filterCleanUp( void );
extern bool jsArgToConfCouple(int nb,CONFcouple **conf,  jsval *argv);
bool A_setVideoCodec(const char *nm);


JSPropertySpec ADM_JSAvidemuxVideo::avidemuxvideo_properties[] = 
{ 
       
	{ 0 }
};

JSFunctionSpec ADM_JSAvidemuxVideo::avidemuxvideo_methods[] = 
{
	{ "clear", Clear, 0, 0, 0 },	// clear
    { "clearFilters", ClearFilters, 0, 0, 0 }, // Delete all filters
	{ "addFilter", AddFilter, 10, 0, 0 },	// Add filter to filter chain
	{ "codec", Codec, 1, 0, 0 },	// Set the video codec
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
/*********************************/
JSPropertySpec *ADM_JsVideoGetProperties(void)
{
    return ADM_JSAvidemuxVideo::avidemuxvideo_properties;
}
JSFunctionSpec *ADM_JsVideoGetFunctions(void)
{
    return ADM_JSAvidemuxVideo::avidemuxvideo_methods;
}
/*********************************/
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


JSBool ADM_JSAvidemuxVideo::ClearFilters(JSContext *cx, JSObject *obj, uintN argc,
                                       jsval *argv, jsval *rval)
{// begin Clear
//	filterCleanUp();
        return JS_TRUE;
}// end Clear

/**
    \fn AddFilter
*/
JSBool ADM_JSAvidemuxVideo::AddFilter(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin AddFilter

        uint32_t filterTag;

        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc == 0)
                return JS_FALSE;
        char *filterName=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        filterTag = ADM_vf_getTagFromInternalName(filterName);
        ADM_info("Adding Filter %s -> %"LU"... \n",filterName,filterTag);

        enterLock();
        CONFcouple *c=NULL;
        if(argc)
            jsArgToConfCouple(argc-1,&c,argv+1);
        *rval=BOOLEAN_TO_JSVAL(  ADM_vf_addFilterFromTag(filterTag,c,false));
        if(c) delete c;
        leaveLock();
        return JS_TRUE;
}// end AddFilter
/**
    \fn Codec
*/
JSBool ADM_JSAvidemuxVideo::Codec(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin Codec
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc <1)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false )
        {
                ADM_error("Cannot set codec\n");
                return JS_FALSE;
        }
        char *codec=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        // Set codec.
        enterLock();
        if(A_setVideoCodec(codec)==false)
        {
            ADM_error("Could not select codec %s\n",codec);
            leaveLock();
            return JS_FALSE;
        }
        CONFcouple *c;
        jsArgToConfCouple(argc-1,&c,argv+1);
        *rval = BOOLEAN_TO_JSVAL( videoEncoder6_SetConfiguration(c));
        ADM_info("Selected codec %s\n",codec);
        if(c) delete c;
        leaveLock();

        return JS_TRUE;
}// end Codec

/**
    \fn Save
*/
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
/**
    \fn saveJpeg
*/
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
/**
    \fn ListBlackFrames
*/
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
/**
    \fn PostProcess
*/
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
/**
    \fn GetFps1000
*/
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
/**
    \fn GetNbFrames
*/
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
/**
    \fn SetFps1000
*/
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

/**
    \fn GetWidth
*/
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
/**
    \fn SetFps1000
*/
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
/**
    \fn GetFCC
*/

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
/**
    \fn SetFps1000
*/

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
/**
    \fn hasGmc
*/

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
/**
    \fn hasQpel
*/

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

/**
    \fn getFrameSize
*/

JSBool ADM_JSAvidemuxVideo::getFrameSize(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
uint32_t info;
uint32_t frame;
uint32_t sz;
        if(argc != 1)
          return JS_FALSE;
#if 0        
        enterLock();
        frame=JSVAL_TO_INT(argv[0]);
        if(!video_body->getFrameSize(frame,&sz)) return JS_FALSE;
        leaveLock(); 
        
        *rval=INT_TO_JSVAL(sz);
#endif
        return JS_TRUE;
}// end PostProcess
/**
    \fn getFrameType
*/

JSBool ADM_JSAvidemuxVideo::getFrameType(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
uint32_t info;
uint32_t frame;
uint32_t sz;
        if(argc != 1)
          return JS_FALSE;
#if 0        
        enterLock();
        frame=JSVAL_TO_INT(argv[0]);
        if(!video_body->getFlags(frame,&sz)) return JS_FALSE;
        leaveLock(); 
        
        *rval=INT_TO_JSVAL(sz);
#endif
        return JS_TRUE;
}// end PostProcess

/**
    \fn A_setVideoCodec
*/
bool A_setVideoCodec(const char *nm)
{
    int idx=videoEncoder6_GetIndexFromName(nm);
    if(idx==-1)
    {
        ADM_error("No such encoder :%s\n",nm);
    }
    // Select by index
    videoEncoder6_SetCurrentEncoder(idx);
    UI_setVideoCodec(idx);
    return true;
}

/* EOF */
