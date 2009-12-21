/****************************************************************************
 copyright            : (C) 2007 by mean
 email                : fixounet@free.fr
 
 Simple class to do filter that have a configuration that have real time effect
 
 Case 1: The filter process YUV
 
 YUV-> Process->YUVOUT->YUV2RGB->RGBUFFEROUT
 
 Case 2: The filter process RGV
 
 YUV-> YUV2RGB->RGB->Process->RGBUFFEROUT
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_FLY_DIALOG_H
#define ADM_FLY_DIALOG_H

#if !defined(ADM_FLY_INTERNAL)
#if !defined(ADM_UI_TYPE_BUILD)
#error No ADM_UI_TYPE_BUILD defined
#endif
#include "DIA_uiTypes.h"

#if ADM_UI_TYPE_BUILD == ADM_UI_QT4
        #include <QtGui/QWidget>
        #include <QtGui/QDialog>
#endif //UI_BUILD_TYPE
#endif //ADM_FLY_INTERNAL 

#include "ADM_default.h"
#include "ADM_colorspace.h"
#include "ADM_coreVideoFilter.h"

#define ADM_FLY_SLIDER_MAX 1000

enum ResizeMethod {
    RESIZE_NONE = 0,	// No automatic resize
	RESIZE_AUTO = 1,	// Resize image when convenient (YUV: after filter, RGB: before applying filter)
	RESIZE_LAST = 2		// Resize image after filter has been applied (slower for RGB)
};

class diaMenuEntry;  // defined in DIA_factory.h; only need pointer here

struct MenuMapping
{
    const char * widgetName; // name of the combo box widget or equivalent
    uint32_t paramOffset;   // offsetof(FOO_PARAM, menu_option_member)
    uint32_t count;
    const diaMenuEntry * menu;
};

typedef float gfloat;
/**
    \class ADM_flyDialog
    \brief Base class for flyDialog
*/
class ADM_flyDialog
{
  protected:
          uint32_t      _w, _h, _zoomW, _zoomH;
          float         _zoom;
          uint32_t      _zoomChangeCount;
          ADM_coreVideoFilter *_in;
      
          ADMImage      *_yuvBuffer;
          ADMImage      *_yuvBufferOut;
          uint8_t       *_rgbBuffer;
          uint8_t       *_rgbBufferOut;
          uint8_t       *_rgbBufferDisplay;
          uint8_t       _isYuvProcessing;
          ResizeMethod  _resizeMethod;
          ADMImageResizer *_resizer;

  
          void EndConstructor(void);
          void copyYuvFinalToRgb(void);
          void copyYuvScratchToRgb(void);
          void copyRgbFinalToDisplay(void);
          
  public:
          void recomputeSize(void);
         
  public:
          void    *_cookie; // whatever
          void    *_slider; // widget
          void    *_canvas; // Drawing zone
          ColYuvRgb *_rgb;

          /* Filter dependant */
  virtual uint8_t    process(void)=0;
  virtual uint8_t    download(void)=0;
  virtual uint8_t    upload(void)=0;
          /* /filter dependant */
  
        /* This is GTK/QT/whatever dependant */
          
  
          
          
  
  virtual uint8_t  update(void) {};
            uint8_t  cleanup(void);
  

#ifdef USE_JOG
  static void jogDial (void * my_data, signed short offset);
  static void jogRing (void * my_data, gfloat angle);
#endif

          ADM_flyDialog(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                             void *canvas, void *slider, int yuv, ResizeMethod resizeMethod);
  virtual ~ADM_flyDialog(void);
// UI dependant part
// They are not defined as pure to avoid unresolved problem, especially on win32.
// You should never use flyDialog as is, but using the macro to pull flyDialogGtk/flyDialogQt4/... 
  
  virtual uint8_t  isRgbInverted(void)=0;
  virtual uint8_t  display(void)=0;
  virtual float    calcZoomFactor(void)=0;
  virtual uint32_t sliderGet(void)=0;             // Return the slider value between 0 and ADM_FLY_SLIDER_MAX
  virtual uint8_t  sliderSet(uint32_t value) =0;  // Set slider value between 0 and ADM_FLY_SLIDE_MAX
  virtual void     postInit(uint8_t reInit)=0;
  virtual uint8_t  sliderChanged(void);
};
#if !defined(ADM_FLY_INTERNAL)

#ifdef ADM_UI_TYPE_BUILD
#if ADM_UI_TYPE_BUILD == ADM_UI_QT4
  #include "DIA_flyDialogQt4.h"
  #define FLY_DIALOG_TYPE ADM_flyDialogQt4
#else 
  #if ADM_UI_TYPE_BUILD == ADM_UI_GTK
    #include "DIA_flyDialogGtk.h"
    #define FLY_DIALOG_TYPE ADM_flyDialogGtk
  #else 
    #if ADM_UI_TYPE_BUILD == ADM_UI_CLI
      #include "DIA_flyDialogCli.h"
      #define FLY_DIALOG_TYPE ADM_flyDialogCli
    #else
        #if ADM_UI_TYPE_BUILD != ADM_UI_NONE
#error unknown UI type
        #endif
    #endif //CLI
  #endif //GTK
#endif // QT
#else
  #define FLY_DIALOG_TYPE ADM_UI_TYPE_BUILD_IS_NOT_DEFINED
#endif
 
#endif //  ADM_FLY_INTERNAL

#endif
