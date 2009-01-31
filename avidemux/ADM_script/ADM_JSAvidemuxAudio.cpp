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
#include "config.h"
#include "ADM_JSGlobal.h"
#include "ADM_JSAvidemuxAudio.h"

#include "../ADM_userInterfaces/ADM_commonUI/GUI_ui.h"
#include "ADM_audiofilter/audioeng_buildfilters.h"
#include "avi_vars.h"
#include "gui_action.hxx"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_videoFilter.h"
#include "ADM_encoder/adm_encoder.h"
#include "ADM_encoder/adm_encConfig.h"
#include "ADM_editor/ADM_outputfmt.h"



#include "ADM_script/ADM_container.h"

extern int A_audioSave(char *name);
extern int A_loadAC3 (char *name);
extern int A_loadMP3 (char *name);
extern int A_loadWave (char *name);
extern void HandleAction(Action act);
extern uint8_t A_setSecondAudioTrack(const AudioSource nw,char *name);

JSPropertySpec ADM_JSAvidemuxAudio::avidemuxaudio_properties[] = 
{ 

        { "process", audioprocess_prop, JSPROP_ENUMERATE },        // process audio when saving
        { "resample", resample_prop, JSPROP_ENUMERATE },	// resample
        { "delay", delay_prop, JSPROP_ENUMERATE },	// set audio delay
        { "film2pal", film2pal_prop, JSPROP_ENUMERATE },	// convert film to pal
        { "pal2film", pal2film_prop, JSPROP_ENUMERATE },	// convert pal to film
        { "normalizeMode", normalizemode_prop, JSPROP_ENUMERATE },	//
        { "drc", drc_prop, JSPROP_ENUMERATE },	//
        { "normalizeValue", normalizevalue_prop, JSPROP_ENUMERATE },	//
        { 0 }
};

JSFunctionSpec ADM_JSAvidemuxAudio::avidemuxaudio_methods[] = 
{
        { "scanVBR", ScanVBR, 0, 0, 0 },	// scan variable bit rate audio
        { "save", Save, 1, 0, 0 },	// save audio stream
        { "load", Load, 2, 0, 0 },	// load audio stream
        { "reset", Reset, 0, 0, 0 },	// reset audio stream
        { "codec", Codec, 4, 0, 0 },	// set output codec
        { "getNbTracks", getNbTracks, 0, 0, 0 },    // set output codec
        { "setTrack", setTrack, 1, 0, 0 },    // set output codec
        { "secondAudioTrack", secondAudioTrack, 2, 0, 0 },    // set audio track
        { "mixer", mixer, 1, 0, 0 },    // set mixer configuration
        { "getNbChannels", getNbChannels, 1, 0, 0 },
        { "getBitrate", getBitrate, 1, 0, 0 },
        { 0 }
};

JSClass ADM_JSAvidemuxAudio::m_classAvidemuxAudio = 
{
        "AvidemuxAudio", JSCLASS_HAS_PRIVATE,
        JS_PropertyStub, JS_PropertyStub,
        ADM_JSAvidemuxAudio::JSGetProperty, ADM_JSAvidemuxAudio::JSSetProperty,
        JS_EnumerateStub, JS_ResolveStub, 
        JS_ConvertStub, ADM_JSAvidemuxAudio::JSDestructor
};

ADM_JSAvidemuxAudio::~ADM_JSAvidemuxAudio(void)
{
        if(m_pObject != NULL)
                delete m_pObject;
        m_pObject = NULL;
}

void ADM_JSAvidemuxAudio::setObject(ADM_AvidemuxAudio *pObject)
{
        m_pObject = pObject; 
}
        
ADM_AvidemuxAudio *ADM_JSAvidemuxAudio::getObject()
{
        return m_pObject; 
}

JSObject *ADM_JSAvidemuxAudio::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
        JSObject *newObj = JS_InitClass(cx, obj, proto, &m_classAvidemuxAudio, 
                                                                        ADM_JSAvidemuxAudio::JSConstructor, 0,
                                                                        ADM_JSAvidemuxAudio::avidemuxaudio_properties, ADM_JSAvidemuxAudio::avidemuxaudio_methods,
                                                                        NULL, NULL);
        return newObj;
}

JSBool ADM_JSAvidemuxAudio::JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
                                                                jsval *argv, jsval *rval)
{
        if(argc != 0)
                return JS_FALSE;
        ADM_JSAvidemuxAudio *p = new ADM_JSAvidemuxAudio();
        ADM_AvidemuxAudio *pObject = new ADM_AvidemuxAudio();
        p->setObject(pObject);
        if ( ! JS_SetPrivate(cx, obj, p) )
                return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
}

void ADM_JSAvidemuxAudio::JSDestructor(JSContext *cx, JSObject *obj)
{
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        if(p != NULL)
                delete p;
        p = NULL;
}

JSBool ADM_JSAvidemuxAudio::JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
        if (JSVAL_IS_INT(id)) 
        {
                ADM_JSAvidemuxAudio *priv = (ADM_JSAvidemuxAudio *) JS_GetPrivate(cx, obj);
                switch(JSVAL_TO_INT(id))
                {
                        case audioprocess_prop:
                                *vp = BOOLEAN_TO_JSVAL(priv->getObject()->m_bAudioProcess);
                                break;
                        case resample_prop:
                                *vp = INT_TO_JSVAL(priv->getObject()->m_nResample);
                                break;
                        case delay_prop:
                                *vp = INT_TO_JSVAL(priv->getObject()->m_nDelay);
                                break;
                        case film2pal_prop:
                                *vp = BOOLEAN_TO_JSVAL(priv->getObject()->m_bFilm2PAL);
                                break;
                        case pal2film_prop:
                                *vp = BOOLEAN_TO_JSVAL(priv->getObject()->m_bPAL2Film);
                                break;
                        case normalizemode_prop:
                              *vp = BOOLEAN_TO_JSVAL(priv->getObject()->m_nNormalizeMode);
                              break;
                        case normalizevalue_prop:
                          *vp = BOOLEAN_TO_JSVAL(priv->getObject()->m_nNormalizeValue);
                          break;
                        case drc_prop:
                            *vp = BOOLEAN_TO_JSVAL(priv->getObject()->m_bDRC);
                            break;
/*
                        case audio_prop:
                                *vp = OBJECT_TO_JSVAL(priv->getObject()->m_pAudio);
                                break;
*/
                }
        }
        return JS_TRUE;
}

JSBool ADM_JSAvidemuxAudio::JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
        if (JSVAL_IS_INT(id)) 
        {
                
                ADM_JSAvidemuxAudio *priv = (ADM_JSAvidemuxAudio *) JS_GetPrivate(cx, obj);
                switch(JSVAL_TO_INT(id))
                {
                        case audioprocess_prop:
                        {
                                if(JSVAL_IS_BOOLEAN(*vp) == false)
                                        break;
                                priv->getObject()->m_bNormalize = JSVAL_TO_BOOLEAN(*vp);
                                enterLock();
                                UI_setAProcessToggleStatus(priv->getObject()->m_bAudioProcess);
                                leaveLock();
                                break;
                        }
                        case drc_prop:
                        {
                                if(JSVAL_IS_BOOLEAN(*vp) == false)
                                        break;
                                priv->getObject()->m_bDRC = JSVAL_TO_BOOLEAN(*vp);
                                enterLock();
                                audioFilterDrc(priv->getObject()->m_bDRC);
                                leaveLock();
                                break;
                        }
                        case resample_prop:
                        {
                                if(JSVAL_IS_INT(*vp) == false)
                                        break;
                                priv->getObject()->m_nResample = JSVAL_TO_INT(*vp);
                                enterLock();
                                audioFilterResample(priv->getObject()->m_nResample);
                                leaveLock();
                                break;
                        }
                        case delay_prop:
                        {
                                if(JSVAL_IS_INT(*vp) == false)
                                        break;
                                priv->getObject()->m_nDelay = JSVAL_TO_INT(*vp);
                                //audioFilterDelay(priv->getObject()->m_nDelay);
                                enterLock();
                                //UI_setTimeShift(1, priv->getObject()->m_nDelay); 
                                leaveLock();
                                break;
                        }
                        case film2pal_prop:
                        {
                                if(JSVAL_IS_BOOLEAN(*vp) == false)
                                        break;
                                priv->getObject()->m_bFilm2PAL = JSVAL_TO_BOOLEAN(*vp);
                                enterLock();
                                audioFilterFilm2Pal(priv->getObject()->m_bFilm2PAL);
                                leaveLock();
                                break;
                        }
                        case pal2film_prop:
                        {
                                if(JSVAL_IS_BOOLEAN(*vp) == false)
                                        break;
                                priv->getObject()->m_bPAL2Film = JSVAL_TO_BOOLEAN(*vp);
                                enterLock();
                                audioFilterPal2Film(priv->getObject()->m_bPAL2Film);
                                leaveLock();
                                break;
                        }
                        case normalizemode_prop:
                        {
                                  enterLock();
                                  priv->getObject()->m_nNormalizeMode = JSVAL_TO_INT(*vp);
                                  audioFilterNormalizeMode(priv->getObject()->m_nNormalizeMode);
                                  leaveLock();
                                  break;
                        }
                        case normalizevalue_prop:
                        {
                                  priv->getObject()->m_nNormalizeValue = JSVAL_TO_INT(*vp);
                                  enterLock();
                                  audioFilterNormalizeValue(priv->getObject()->m_nNormalizeValue);
                                  leaveLock();
                                  break;
                        }
                        default : printf("UNKNOWN AUDIO PROP\n");
                        return JS_FALSE;
                }
        }
        return JS_TRUE;
}

JSBool ADM_JSAvidemuxAudio::ScanVBR(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{// begin ScanVBR
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc != 0)
                return JS_FALSE;
        printf("Scaning Audio... \n");
        enterLock();
        HandleAction(ACT_AudioMap);
        leaveLock()
        *rval = BOOLEAN_TO_JSVAL(true);
        return JS_TRUE;
}// end ScanVBR

JSBool ADM_JSAvidemuxAudio::Save(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{// begin Save
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc != 1)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false)
                return JS_FALSE;
        char *pTempStr = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        printf("Saving Audio \"%s\"\n",pTempStr);
        enterLock();
        *rval = INT_TO_JSVAL(A_audioSave(pTempStr));
        leaveLock();
        return JS_TRUE;
}// end Save

JSBool ADM_JSAvidemuxAudio::Load(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{// begin Load
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc != 2)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false || JSVAL_IS_STRING(argv[1]) == false)
                return JS_FALSE;
        char *pTempStr = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
        char *pArg0 = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        printf("Loading Audio \"%s\"\n",pTempStr);

        // 1st arg is type
        AudioSource src;
        int result=0;
        
        src=audioSourceFromString(pArg0);
        if(!src) {printf("[Script]Invalid audiosource type\n");return JS_FALSE;}
        enterLock();
        switch(src)
        {
                case AudioAvi:
                        result = A_changeAudioStream (aviaudiostream, AudioAvi,NULL);
                        break;
                case AudioMP3:
                        result = A_loadMP3(pTempStr);
                        break;
                case AudioWav:
                        result = A_loadWave(pTempStr);
                        break;
                case AudioAC3:
                        result = A_loadAC3(pTempStr);
                        break;
                case AudioNone:
                        result = A_changeAudioStream(NULL,AudioNone,NULL);
                        break;
                default:
                        ADM_assert(0);
                        break;
        }
        leaveLock()
        printf("[script] ");
        
        if(!result)
                printf("failed :");
        else
                printf("succeed :");
        printf(" external source %d (%s) \n", src,pTempStr);

        *rval = INT_TO_JSVAL(result);
        return JS_TRUE;
}// end Load

JSBool ADM_JSAvidemuxAudio::Reset(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{// begin Reset
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc != 0)
                return JS_FALSE;
        enterLock();
        audioReset();
        leaveLock()
        *rval = BOOLEAN_TO_JSVAL(true);
        return JS_TRUE;
}// end Reset
// app.audio.codec("lame",128,8,"00 00 00 00 01 00 00 00 ");
extern uint8_t mk_hex (uint8_t a, uint8_t b);
JSBool ADM_JSAvidemuxAudio::Codec(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{// begin Codec
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc != 4)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false || JSVAL_IS_INT(argv[1]) == false || JSVAL_IS_INT(argv[2]) == false  ||  JSVAL_IS_STRING(argv[3]) == false  )
                return JS_FALSE;
        char *name = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        ADM_LowerCase(name);
        enterLock();
        // First search the codec by its name
        if(!audioCodecSetByName(name))
                *rval = BOOLEAN_TO_JSVAL(false);
        else
        {// begin set bitrate
        //audioFilter_SetBitrate(JSVAL_TO_INT(argv[1]));
          uint32_t bitrate,size;
          char  *extra;
          uint8_t *data=NULL;
                bitrate=JSVAL_TO_INT(argv[1]);
                size=JSVAL_TO_INT(argv[2]);
                extra=JS_GetStringBytes(JSVAL_TO_STRING(argv[3])); 
                if(size)
                {
                          data=new uint8_t[size];
                          for (uint32_t k = 0; k < size; k++)
                            {
                              data[k] = mk_hex (*extra, *(extra + 1));
                              extra += 3;
                            }
                        setAudioExtraConf(bitrate,size,data);
                        delete [] data; 
                }

                *rval = BOOLEAN_TO_JSVAL(true);
        }// end set bitrate
        leaveLock();
        return JS_TRUE;
}// end Codec
JSBool ADM_JSAvidemuxAudio::getNbTracks(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{
uint32_t nb=0;
audioInfo *infos=NULL;
        // default return value
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        if(argc != 0)
                return JS_FALSE;
        // default return value
      
        enterLock();
        video_body->getAudioStreamsInfo(0,&nb, &infos);
        leaveLock();
        if(infos)
                delete [] infos;
        *rval = INT_TO_JSVAL(nb);
        return JS_TRUE;
}// end Codec
JSBool ADM_JSAvidemuxAudio::setTrack(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{
uint32_t nb=0,nw=0;
audioInfo *infos=NULL;
        // default return value
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);

        // default return value
      if(argc != 1)
                return JS_FALSE;
        if(JSVAL_IS_INT(argv[0]) == false)
                return JS_FALSE;
        video_body->getAudioStreamsInfo(0,&nb, &infos);
        delete [] infos;
        nw=(JSVAL_TO_INT(argv[0]));
        if(nw>nb) return JS_FALSE;
        enterLock();
        video_body->changeAudioStream(0,nw);
        leaveLock();
        return JS_TRUE;
}// end Codec
JSBool ADM_JSAvidemuxAudio::secondAudioTrack(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        if(argc != 2)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false || JSVAL_IS_STRING(argv[1]) == false)
                return JS_FALSE;
        // First arg is MP3 etc...
        char *name = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        ADM_LowerCase(name);
        // First search the codec by its name
        AudioSource source;
        if(AudioInvalid==(source=audioSourceFromString(name)))
                return JS_FALSE;
        // Now get the name
        name = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
        enterLock();
        if(A_setSecondAudioTrack(source,name))
        {
                leaveLock();
                return JS_TRUE;
        }
        leaveLock();
      return JS_FALSE;
}
JSBool ADM_JSAvidemuxAudio::mixer(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{
uint32_t nb=0,nw=0;
uint32_t *infos=NULL;
        // default return value
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);

        // default return value
      if(argc != 1)
                return JS_FALSE;
        char *pArg0 = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        enterLock();

        if(setCurrentMixerFromString(pArg0))
                *rval=BOOLEAN_TO_JSVAL(true);
        else
                *rval=BOOLEAN_TO_JSVAL(false);
        leaveLock();
        return JS_TRUE;

}// end Codec

JSBool ADM_JSAvidemuxAudio::getNbChannels(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{
uint32_t nb=0, nw=0;
audioInfo *infos=NULL;
        // default return value
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        if(argc != 1)
                return JS_FALSE;
        // default return value
      
        if(JSVAL_IS_INT(argv[0]) == false)
                return JS_FALSE;
        enterLock();
        video_body->getAudioStreamsInfo(0,&nb, &infos);
        leaveLock()
        nw=(JSVAL_TO_INT(argv[0]));
        if(nw>nb)
                return JS_FALSE;
        *rval = INT_TO_JSVAL(infos[nw].channels);
        delete [] infos;
        return JS_TRUE;
}// end Codec

JSBool ADM_JSAvidemuxAudio::getBitrate(JSContext *cx, JSObject *obj, uintN argc, 
                                      jsval *argv, jsval *rval)
{
uint32_t nb=0, nw=0;
audioInfo *infos=NULL;
        // default return value
        ADM_JSAvidemuxAudio *p = (ADM_JSAvidemuxAudio *)JS_GetPrivate(cx, obj);
        if(argc != 1)
                return JS_FALSE;
        // default return value
      
        if(JSVAL_IS_INT(argv[0]) == false)
                return JS_FALSE;
        enterLock();
        video_body->getAudioStreamsInfo(0,&nb, &infos);
        leaveLock()
        nw=(JSVAL_TO_INT(argv[0]));
        if(nw>nb)
                return JS_FALSE;
        *rval = INT_TO_JSVAL(infos[nw].bitrate);
        delete [] infos;
        return JS_TRUE;
}// end Codec

