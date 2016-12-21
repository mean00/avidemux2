/***************************************************************************
                          DIA_coreUI_internal.h
  Handles univeral dialog
  (C) Mean 2006 fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DIA_COREUI_INTERNAL_H
#define DIA_COREUI_INTERNAL_H

#include "ADM_coreUI6_export.h"
#include "DIA_enter.h"
#include "DIA_fileSel.h"
#include "DIA_factory.h"
#include "DIA_working.h"
#include "DIA_processing.h"
#include "DIA_encoding.h"
#include "DIA_audioTracks.h"

#define ADM_CORE_TOOLKIT_MAJOR 2
#define ADM_CORE_TOOLKIT_MINOR 0
// Dia enter
typedef struct
{
    DIA_FILE_INIT      *fileInit;
    DIA_FILE_SEL_CB   *fileReadCb;
    DIA_FILE_SEL_CB   *fileWriteCb;
    DIA_FILE_SEL_NAME *fileReadName;
    DIA_FILE_SEL_NAME *fileWriteName;
    DIA_FILE_SELECT   *fileSelectRead;
    DIA_FILE_SELECT   *fileSelectWrite;
    DIA_FILE_SELECT   *fileSelectDirectory;
    DIA_FILE_SEL_CB_EXTENSION *fileSelWriteWithExtension;
    DIA_FILE_SEL_CB_EXTENSION *fileSelReadWithExtension;
}DIA_FILESEL_DESC_T;

// Call this to hook your fileSelector functions
ADM_COREUI6_EXPORT uint8_t DIA_fileSelInit(DIA_FILESEL_DESC_T *d);


//
typedef uint8_t DIA_FACTORY_RUN_TABS(const char *title,uint32_t nb,diaElemTabs **tabs);
typedef void  * DIA_FACTORY_RUN_TABS_PREPARE(const char *title,uint32_t nb,diaElemTabs **tabs);
typedef bool  DIA_FACTORY_RUN_TABS_FINISH(void *foo);
typedef uint8_t DIA_FACTORY_RUN(const char *title,uint32_t nb,diaElem **elems);
typedef void   *DIA_FACTORY_RUN_PREPARE(const char *title,uint32_t nb,diaElem **elems);
typedef bool  DIA_FACTORY_RUN_FINISH(void *foo);

typedef struct
{
    COREUI_GET_VERSION   *FactoryGetVersion;
    DIA_FACTORY_RUN      *FactoryRun;
    DIA_FACTORY_RUN_TABS *FactoryRunTab;
    DIA_FACTORY_RUN_TABS_PREPARE *FactoryTabPrepare;
    DIA_FACTORY_RUN_TABS_FINISH  *FactoryTabFinish;
    DIA_FACTORY_RUN_PREPARE      *FactoryPrepare;
    DIA_FACTORY_RUN_FINISH       *FactoryFinish;
        
//    
    CREATE_BUTTON_T      *CreareButton;
    DELETE_DIA_ELEM_T    *DestroyButton;
//
    CREATE_BAR_T         *CreateBar;
    DELETE_DIA_ELEM_T    *DestroyBar;
// Float
    CREATE_FLOAT_T       *CreateFloat;
    DELETE_DIA_ELEM_T    *DestroyFloat;
// Integer    
    CREATE_INTEGER_T     *CreateInteger;
    DELETE_DIA_ELEM_T    *DestroyInteger;
// UInteger    
    CREATE_UINTEGER_T    *CreateUInteger;
    DELETE_DIA_ELEM_T    *DestroyUInteger;
// Notch    
    CREATE_NOTCH_T       *CreateNotch;
    DELETE_DIA_ELEM_T    *DestroyNotch;
// ReadonlyText
    CREATE_READONLYTEXT_T *CreateReadonlyText;
    DELETE_DIA_ELEM_T     *DestroyReadonlyText;
// Text
    CREATE_TEXT_T        *CreateText;
    DELETE_DIA_ELEM_T    *DestroyText;
// Hex
    CREATE_HEX_T         *CreateHex;
    DELETE_DIA_ELEM_T    *DestroyHex;
// Matrix
    CREATE_MATRIX_T      *CreateMatrix;
    DELETE_DIA_ELEM_T    *DestroyMatrix;
// Menu
    CREATE_MENU_T        *CreateMenu;
    DELETE_DIA_ELEM_T    *DestroyMenu;
// MenuDynamic
    CREATE_MENUDYNAMIC_T *CreateMenuDynamic;
    DELETE_DIA_ELEM_T    *DestroyMenuDynamic;
// ThreadCount
    CREATE_THREADCOUNT_T *CreateThreadCount;
    DELETE_DIA_ELEM_T    *DestroyThreadCount;
// Bitrate
    CREATE_BITRATE_T     *CreateBitrate;
    DELETE_DIA_ELEM_T    *DestroyBitrate;
// File
    CREATE_FILE_T        *CreateFile;
    DELETE_DIA_ELEM_T    *DestroyFile;
// Dir
    CREATE_DIR_T         *CreateDir;
    DELETE_DIA_ELEM_T    *DestroyDir;
// Frame
    CREATE_FRAME_T       *CreateFrame;
    DELETE_DIA_ELEM_T    *DestroyFrame;
// Toggle uint
    CREATE_TOGGLE_UINT   *CreateToggleUint;
    DELETE_DIA_ELEM_T    *DestroyToggleUint;
// Toggle int
    CREATE_TOGGLE_INT    *CreateToggleInt;
    DELETE_DIA_ELEM_T    *DestroyToggleInt;
// Regular Toggle    
    CREATE_TOGGLE_T      *CreateToggle;
    DELETE_DIA_ELEM_T    *DestroyToggle;
// Uslider
    CREATE_USLIDER_T     *CreateUSlider;
    DELETE_DIA_ELEM_T    *DestroyUSlider;
// Slider
    CREATE_SLIDER_T      *CreateSlider;
    DELETE_DIA_ELEM_T    *DestroySlider;
// TimeStamp
    CREATE_TIMESTAMP_T   *CreateTimeStamp;
    DELETE_DIA_ELEM_T    *DestroyTimeStamp;
// Aspect Ratio 
    CREATE_ASPECTRATIO_T *CreateAspectRatio;
    DELETE_DIA_ELEM_T    *DestroyAspectRatio;

}FactoryDescriptor;
//
ADM_COREUI6_EXPORT uint8_t DIA_factoryInit(FactoryDescriptor *d);

// This is for coreToolkit UI elements
typedef void            GET_CORE_TOOLKIT_VERSION(uint32_t *maj, uint32_t *minor);
typedef void            CREATE_GUI_INFO_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format);
typedef void            CREATE_GUI_ERROR_HIG(const char *primary, const char *secondary_format);
typedef int             CREATE_GUI_CONFIRMATION_HIG(const char *button_confirm, const char *primary, const char *secondary_format);
typedef int             CREATE_GUI_YESNO(const char *primary, const char *secondary_format);
typedef int             CREATE_GUI_QUESTION(const char *alertstring);
typedef int             CREATE_GUI_ALTERNATE(const char *title,const char *choice1,const char *choice2);
typedef void            CREATE_GUI_VERBOSE(void);
typedef void            CREATE_GUI_QUIET(void);
typedef uint8_t        CREATE_GUI_IS_GUIET(void);
typedef DIA_workingBase    *CREATE_GUI_WORKING(const char *title);
typedef DIA_processingBase *CREATE_GUI_PROCESSING(const char *title,uint64_t totalToProcess);
typedef DIA_encodingBase   *CREATE_GUI_ENCODING(uint64_t duration);
typedef DIA_audioTrackBase *CREATE_GUI_AUDIOTRACKBASE(PoolOfAudioTracks * pool,ActiveAudioTracks *active);
typedef void             UI_PURGE(void);

typedef struct
{
    GET_CORE_TOOLKIT_VERSION    *getVersion;
    CREATE_GUI_INFO_HIG         *infoHig;
    CREATE_GUI_ERROR_HIG        *errorHig;
    CREATE_GUI_CONFIRMATION_HIG *confirmationHig;
    CREATE_GUI_YESNO            *yesno;
    CREATE_GUI_QUESTION         *question;
    CREATE_GUI_ALTERNATE        *alternate;
    CREATE_GUI_VERBOSE          *verbose;
    CREATE_GUI_QUIET            *quiet;
    CREATE_GUI_IS_GUIET         *isQuiet;
    CREATE_GUI_WORKING          *createWorking;
    CREATE_GUI_ENCODING         *createEncoding;
    CREATE_GUI_AUDIOTRACKBASE   *createAudioTrack;
    UI_PURGE                    *uiPurge;
    CREATE_GUI_PROCESSING       *createProcessing;
    
}CoreToolkitDescriptor;
//
ADM_COREUI6_EXPORT uint8_t  DIA_toolkitInit(CoreToolkitDescriptor *d);
#endif
