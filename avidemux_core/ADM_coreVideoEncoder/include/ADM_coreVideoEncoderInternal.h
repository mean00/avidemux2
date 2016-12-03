/**
    \file  ADM_coreVideoEncoderInternal.h
    \brief interface to video encoder plugins

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef VIDEOENCODERINTERNAL_H
#define VIDEOENCODERINTERNAL_H

#define ADM_VIDEO_ENCODER_API_VERSION 7

#include "ADM_coreVideoEncoder6_export.h"
#include "BVector.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_dynamicLoading.h"
#include "DIA_uiTypes.h"
#include "ADM_paramList.h"

/*!
  This structure defines a video encoder
  \param encoder Encoder attached to this descriptor
  \param name The name of the codec
  \param configure Function to call to configure the codec
  \param param : An opaque structure that contains the codec specific configuration datas
*/
typedef struct
{
    const char   *encoderName;        // Internal name (tag)
    const char   *menuName;         // Displayed name (in menu)
    const char   *description;      // Short description

    uint32_t     apiVersion;            // const
    ADM_coreVideoEncoder *(*create)(ADM_coreVideoFilter *head,bool globalHeader);
    void         (*destroy)(ADM_coreVideoEncoder *codec);
    bool         (*configure)(void);                                // Call UI to set it up
    bool         (*setProfile)(const char *profile);
    const char   *(*getProfile)(void);
    bool         (*getConfigurationData)(CONFcouple **c); // Get the encoder private conf
    bool         (*setConfigurationData)(CONFcouple *c,bool full);   // Set the encoder private conf
    void         (*resetConfigurationData)();
    bool         (*probe)(); // Check if the encoder is usable. It could depend on external sw/hw such as specific video cards 

    ADM_UI_TYPE  UIType;                // Type of UI
    uint32_t     major,minor,patch;     // Version of the plugin

    void         *opaque;               // Hide stuff in here
}ADM_videoEncoderDesc;

/**
    \class ADM_videoEncoder6
    \brief Plugin Wrapper Class

*/
class ADM_videoEncoder6 :public ADM_LibWrapper
{
public:
        int                  initialised;
        ADM_videoEncoderDesc *desc;
        ADM_videoEncoderDesc  *(*getInfo)();
        ADM_videoEncoder6(const char *file) : ADM_LibWrapper()
        {
			initialised = (loadLibrary(file) && getSymbols(1,
				&getInfo, "getInfo"));
                if(initialised)
                {
                    desc=getInfo();
                    printf("[videoEncoder6]Name :%s ApiVersion :%d Description :%s\n",
                                                        desc->encoderName,
                                                        desc->apiVersion,
                                                        desc->description);
                }else
                {
                    printf("[videoEncoder6]Symbol loading failed for %s\n",file);
                }
        }
};

extern ADM_COREVIDEOENCODER6_EXPORT BVector <ADM_videoEncoder6 *> ListOfEncoders;

// Macros to declare audio encoder
/**************************************************************************/
#define ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(Class) \
static bool xgetConfigurationData (CONFcouple **c); \
static bool xsetConfigurationData (CONFcouple *c,bool full=true);\
\
static ADM_coreVideoEncoder * create (ADM_coreVideoFilter * head,bool globalHeader) \
{ \
  return new Class (head,globalHeader); \
} \
static void destroy (ADM_coreVideoEncoder * in) \
{\
  Class *z = (Class *) in; \
  delete z; \
}
//******************************************************

#define ADM_DECLARE_VIDEO_ENCODER_MAIN_EX(name,menuName,desc,configure,uiType,maj,minV,patch,confTemplate,confVar,setProfile,getProfile,probe) \
static ADM_videoEncoderDesc encoderDesc={\
    name,\
    menuName,\
    desc,\
    ADM_VIDEO_ENCODER_API_VERSION,\
    &create,\
    &destroy,\
    configure,\
    setProfile,\
    getProfile, \
    xgetConfigurationData,\
    xsetConfigurationData,\
    resetConfigurationData,\
    probe,\
    uiType,\
    maj,minV,patch,\
    NULL\
};\
bool xgetConfigurationData (CONFcouple **c)\
{\
         if(confTemplate==NULL) {*c=NULL;return true;} \
         return ADM_paramSave(c,confTemplate,confVar); \
}\
bool xsetConfigurationData (CONFcouple *c,bool full)\
{\
	if(full) return ADM_paramLoad(c,confTemplate,confVar); \
	return ADM_paramLoadPartial(c,confTemplate,confVar); \
} \
extern "C" ADM_PLUGIN_EXPORT ADM_PLUGIN_EXPORT ADM_videoEncoderDesc *getInfo (void) \
{ \
  return &encoderDesc; \
}  \

#define ADM_DECLARE_VIDEO_ENCODER_MAIN(name,menuName,desc,configure,uiType,maj,minV,patch,confTemplate,confVar,setProfile,getProfile) \
extern "C" ADM_PLUGIN_EXPORT bool probe (void) \
{ \
  return true; \
}  \
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX(name,menuName,desc,configure,uiType,maj,minV,patch,confTemplate,confVar,setProfile,getProfile,probe) \


#endif
//EOF
