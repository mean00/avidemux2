/****************************************************************************
 copyright            : (C) 2007 by mean
 email                : fixounet@free.fr
 
 Simple class to do filter that have a configuration that have real time effect
 
 Case 1: The filter process YUV
 
 YUV-> Process->YUVOUT->YUV2RGB->RGBUFFEROUT
 
 Case 2: The filter process RGV
 
 YUV-> YUV2RGB->RGB->Process->RGBUFFEROUT

The ADM_flyDialog implments the common part
The yuv/rgb specific part is implement through ADM_flyDialogAction (yuv/rgb)

 
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

#include "ADM_coreVideoFilter6_export.h"

#if !defined(ADM_FLY_INTERNAL)
#if !defined(ADM_UI_TYPE_BUILD)
#error No ADM_UI_TYPE_BUILD defined
#endif
#include "DIA_uiTypes.h"

#if ADM_UI_TYPE_BUILD == ADM_UI_QT4
        #include <QWidget>
        #include <QDialog>
#endif //UI_BUILD_TYPE
#endif //ADM_FLY_INTERNAL 

#include "ADM_default.h"
#include "ADM_rgb.h"
#include "ADM_colorspace.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_imageResizer.h"
#define ADM_FLY_SLIDER_MAX 1000

enum ResizeMethod {
    RESIZE_NONE = 0,	// No automatic resize
	RESIZE_AUTO = 1,	// Resize image when convenient (YUV: after filter, RGB: before applying filter)
	RESIZE_LAST = 2		// Resize image after filter has been applied (slower for RGB)
};

class diaMenuEntry;  // defined in DIA_factory.h; only need pointer here
class ADM_flyDialog;
struct MenuMapping
{
    const char * widgetName; // name of the combo box widget or equivalent
    uint32_t paramOffset;   // offsetof(FOO_PARAM, menu_option_member)
    uint32_t count;
    const diaMenuEntry * menu;
};
/**
    \class ADM_flyDialogAction
*/
class ADM_flyDialogAction
{
protected:
            ADM_flyDialog *parent;
            ADM_flyDialogAction(ADM_flyDialog *parent) {this->parent=parent;}
public:
  virtual   ~ADM_flyDialogAction() {}
  virtual    bool process(void)=0;
  virtual    void resetScaler(void)=0;
};

class ADM_flyDialogActionYuv: public ADM_flyDialogAction
{
protected:
          ADMImage      *_yuvBufferOut;
          ADMColorScalerFull *yuvToRgb;
public:
            ADM_flyDialogActionYuv(ADM_flyDialog *parent);
            ~ADM_flyDialogActionYuv();
        void resetScaler(void);
        bool process(void);
};

class ADM_flyDialogActionRgb: public ADM_flyDialogAction
{
protected:
          ADM_byteBuffer     _rgbByteBuffer;
          ADM_byteBuffer     _rgbByteBufferOut;
          ADMColorScalerFull *yuv2rgb;
          ADMColorScalerFull *rgb2rgb;
public:
            ADM_flyDialogActionRgb(ADM_flyDialog *parent);
            ~ADM_flyDialogActionRgb();
        bool process(void);
        void resetScaler(void);
};
//***************************************
typedef float gfloat;
/**
    \class ADM_flyDialog
    \brief Base class for flyDialog
*/
class ADM_COREVIDEOFILTER6_EXPORT ADM_flyDialog
{
    friend class ADM_flyDialogAction;
    friend class ADM_flyDialogActionYuv;
    friend class ADM_flyDialogActionRgb;
  protected:
   virtual ADM_colorspace toRgbColor(void);
          void          updateZoom(void);
          ADM_flyDialogAction *action;
          uint64_t      _currentPts;
          uint32_t      _w, _h, _zoomW, _zoomH;
          float         _zoom;
          uint32_t      _zoomChangeCount;
          ADM_coreVideoFilter *_in;
      
          ADMImage      *_yuvBuffer;
          ADM_byteBuffer _rgbByteBufferDisplay;


          
          uint8_t       _isYuvProcessing;
          ResizeMethod  _resizeMethod;
  
          void EndConstructor(void);
          
  public:
          void               recomputeSize(void);
          virtual bool       nextImage(void);
          virtual bool       sameImage(void);
  public:
          void    *_cookie; // whatever
          void    *_slider; // widget
          void    *_canvas; // Drawing zone
          
  
  
  /* Filter dependant : you have to implement them*/
  virtual uint8_t    processYuv(ADMImage* in, ADMImage *out) {ADM_assert(0);return 1;}
  virtual uint8_t    processRgb(uint8_t *in, uint8_t *out) {ADM_assert(0);return 1;}
  virtual uint8_t    download(void)=0;
  virtual uint8_t    upload(void)=0;
  virtual bool       setCurrentPts(uint64_t pts)=0;
  /* /filter dependant */
  
  
  virtual uint8_t  update(void) {return 1;};
            uint8_t  cleanup(void);
  

#ifdef USE_JOG
  static void jogDial (void * my_data, signed short offset);
  static void jogRing (void * my_data, gfloat angle);
#endif

          ADM_flyDialog(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                             void *canvas, void *slider, int yuv, 
                             ResizeMethod resizeMethod);
  virtual ~ADM_flyDialog(void);
// UI dependant part : They are implemented in ADM_flyDialogGtk/Qt/...
public:  
  virtual bool     isRgbInverted(void)=0;
  virtual uint8_t  display(uint8_t *rgbData)=0;
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

