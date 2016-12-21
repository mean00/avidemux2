/***************************************************************************
                          DIA_factory.h
  Handles univeral dialog
  
  DO NOT USE TEMPLATE HERE!
  
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
#pragma once
#include <string>
#include "ADM_coreUI6_export.h"
#include "ADM_assert.h"

#define ADM_COREUI_MAJOR 1
#define ADM_COREUI_MINOR 0
#define ADM_COREUI_PATCH 0

typedef enum 
{
  ELEM_INVALID=0,
  ELEM_TOGGLE,
  ELEM_INTEGER,
  ELEM_FLOAT,
  ELEM_MENU,
  ELEM_FILE_READ,
  ELEM_BITRATE,
  ELEM_BAR,
  ELEM_ROTEXT,
  ELEM_NOTCH,
  ELEM_DIR_SELECT,
  ELEM_TEXT,
  ELEM_FRAME,
  ELEM_HEXDUMP,
  ELEM_TOGGLE_UINT,
  ELEM_TOGGLE_INT,
  ELEM_BUTTON,
  ELEM_SLIDER,
  ELEM_THREAD_COUNT,
  ELEM_MATRIX,
  ELEM_COUNT,
  ELEM_ASPECT_RATIO,
  ELEM_TIMESTAMP,
  ELEM_MAX=ELEM_COUNT-1
}elemEnum;
typedef void ADM_FAC_CALLBACK(void *cookie);

#define ELEM_TYPE_FLOAT float

/*********************************************/
class ADM_COREUI6_EXPORT diaElem
{
  protected:
    void    setSize(int z) {size=z;};
    int     readOnly;
    diaElem    *internalPointer;
public:
  void *param;
  void *myWidget;
  const char *paramTitle;
  const char *tip;
  elemEnum mySelf;
  int       size; // Size of the widget in line

  diaElem(elemEnum num) {paramTitle=NULL;param=NULL;mySelf=num;myWidget=NULL;size=1;readOnly=0;internalPointer=NULL;};
          int getSize(void) {return size;};
  virtual ~diaElem() {};
  virtual void setMe(void *dialog, void *opaque,uint32_t line)=0;
  virtual void getMe(void)=0;
  virtual void setRo(void) {readOnly=1;}
  virtual void setRw(void) {readOnly=0;}
          
  virtual void enable(uint32_t onoff) {}
  virtual void finalize(void) {} // in case some widget needs some stuff just before using them
  virtual int getRequiredLayout(void)=0;
};
/*********************************************/
#define MENU_MAX_lINK 10
typedef struct dialElemLink
{
  uint32_t  value;
  uint32_t  onoff;
  diaElem  *widget;
}dialElemLink;
/*********************************************/

typedef void      DELETE_DIA_ELEM_T(diaElem *widget);
typedef void      FINALIZE_DIA_ELEM_T(void);
typedef void      COREUI_GET_VERSION(uint32_t *maj,uint32_t *min,uint32_t *patch);
/*********************************************/
typedef diaElem  *(CREATE_BUTTON_T)(const char *toggleTitle, ADM_FAC_CALLBACK *cb,void *cookie,const char *tip);
class ADM_COREUI6_EXPORT diaElemButton : public diaElem
{
  protected:
  public:
    void            *_cookie;
    ADM_FAC_CALLBACK *_callBack;
            diaElemButton(const char *toggleTitle, ADM_FAC_CALLBACK *cb,void *cookie,const char *tip=NULL);
  virtual   ~diaElemButton() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};

/*********************************************/
typedef diaElem  *(CREATE_MATRIX_T)(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip);

class ADM_COREUI6_EXPORT diaElemMatrix : public diaElem
{
  protected:
  public:
    uint8_t *_matrix;
    uint32_t _matrixSize;
    		diaElemMatrix(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip=NULL);
  virtual   ~diaElemMatrix() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};
/************************************/
class ADM_COREUI6_EXPORT diaElemSliderBase : public diaElem
{
  protected:
    
      uint32_t digits;
public:
	                diaElemSliderBase() : diaElem(ELEM_SLIDER) {}
  virtual           ~diaElemSliderBase() {};
  virtual uint8_t   setDigits(uint32_t digits) { this->digits = digits; return 1;}
};

typedef diaElem *CREATE_USLIDER_T(uint32_t *value,const char *toggleTitle, uint32_t min,uint32_t max,uint32_t incr , const char *tip);
typedef diaElem *CREATE_SLIDER_T(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,int32_t incr , const char *tip);
class ADM_COREUI6_EXPORT diaElemUSlider : public diaElemSliderBase
{
  protected:
	  uint32_t min,max,incr;
public:
	diaElemUSlider(uint32_t *value,const char *toggleTitle, uint32_t min,uint32_t max,uint32_t incr = 1, const char *tip=NULL);
  virtual   ~diaElemUSlider() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  uint8_t   setDigits(uint32_t digits) ;
  int getRequiredLayout(void);
};
class ADM_COREUI6_EXPORT diaElemSlider : public diaElemSliderBase
{
  protected:
    
	  int32_t min,max,incr;
    
public:
	diaElemSlider(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,int32_t incr = 1, const char *tip=NULL);
  virtual   ~diaElemSlider() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  uint8_t   setDigits(uint32_t digits) ;
  int getRequiredLayout(void);
};
/*********************************************/
typedef diaElem *CREATE_TOGGLE(bool *toggleValue,const char *toggleTitle, const char *tip);
class ADM_COREUI6_EXPORT diaElemToggleBase : public diaElem
{
  protected:
    dialElemLink        links[MENU_MAX_lINK];
    uint32_t            nbLink;
    
public:
			diaElemToggleBase() :diaElem(ELEM_TOGGLE)
						{};
  virtual   ~diaElemToggleBase() {};
  virtual uint8_t   link(uint32_t onoff,diaElem *w)=0;
};
typedef diaElem *CREATE_TOGGLE_T(bool *toggleValue,const char *toggleTitle, const char *tip);
class ADM_COREUI6_EXPORT diaElemToggle : public diaElemToggleBase
{
  protected:
public:
            diaElemToggle(bool *toggleValue,const char *toggleTitle, const char *tip=NULL);
  virtual   ~diaElemToggle() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  void      finalize(void);
  void      updateMe();
  uint8_t   link(uint32_t onoff,diaElem *w);
  int getRequiredLayout(void);
};
/*********************************************/
typedef diaElem *CREATE_TOGGLE_UINT(uint32_t *toggleValue,const char *toggleTitle, uint32_t *uintval, 
								const char *name,uint32_t min,uint32_t max,const char *tip);
class ADM_COREUI6_EXPORT diaElemToggleUint : public diaElem
{
  protected:
        uint32_t *emb;
        const char *embName;
        void *widgetUint;
        uint32_t _min,_max;
public:
            diaElemToggleUint(uint32_t *toggleValue,const char *toggleTitle, uint32_t *uintval,
            					const char *name,uint32_t min,uint32_t max,const char *tip=NULL);
  virtual   ~diaElemToggleUint() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  void      finalize(void);
  void      updateMe();
  int getRequiredLayout(void);
};
typedef diaElem *CREATE_TOGGLE_INT(uint32_t *toggleValue,const char *toggleTitle, int32_t *intval, 
									const char *name,int32_t min,int32_t max,const char *tip);
class ADM_COREUI6_EXPORT diaElemToggleInt : public diaElem
{
  protected:
	  		 int32_t *emb;
	         const char *embName;
	         void *widgetUint;
	         int32_t _min,_max;
public:
            diaElemToggleInt(uint32_t *toggleValue,const char *toggleTitle, int32_t *uintval,
            				const char *name,int32_t min,int32_t max,const char *tip=NULL);
  virtual   ~diaElemToggleInt() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      finalize(void);
  int getRequiredLayout(void);
  void      updateMe();
  void      enable(uint32_t onoff) ;
};
/*********************************************/
typedef diaElem  *(CREATE_INTEGER_T)(int32_t *intValue,const char *toggleTitle,int32_t min, int32_t max,const char *tip);
typedef diaElem  *(CREATE_UINTEGER_T)(uint32_t *intValue,const char *toggleTitle,uint32_t min, uint32_t max,const char *tip);
class ADM_COREUI6_EXPORT diaElemInteger : public diaElem
{

public:
  int32_t min,max;
  diaElemInteger(int32_t *intValue,const char *toggleTitle, int32_t min, int32_t max,const char *tip=NULL);
  virtual ~diaElemInteger() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};
/* Same but unsigned */
class ADM_COREUI6_EXPORT diaElemUInteger : public diaElem
{

public:
  uint32_t min,max;
  diaElemUInteger(uint32_t *intValue,const char *toggleTitle, uint32_t min, uint32_t max,const char *tip=NULL);
  virtual ~diaElemUInteger() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};
/*************************************************/
typedef diaElem  *(CREATE_BAR_T)(uint32_t percent,const char *toggleTitle);

class ADM_COREUI6_EXPORT diaElemBar : public diaElem
{
  protected :
        uint32_t per;
public:
  
  diaElemBar(uint32_t percent,const char *toggleTitle);
  virtual ~diaElemBar() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};
/*************************************************/
typedef diaElem  *(CREATE_TIMESTAMP_T)(uint32_t *value,const char *toggleTitle,const uint32_t valMin, const uint32_t valMax);

class ADM_COREUI6_EXPORT diaElemTimeStamp : public diaElem
{
  protected :
        uint32_t value;
public:
  
  diaElemTimeStamp(uint32_t *value,const char *toggleTitle,const uint32_t valMin, const uint32_t valMax);
  virtual ~diaElemTimeStamp() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};


/*********************************************/
typedef diaElem  *(CREATE_FLOAT_T)(ELEM_TYPE_FLOAT *intValue,const char *toggleTitle, ELEM_TYPE_FLOAT min,
        ELEM_TYPE_FLOAT max,const char *tip, int decimals);
class ADM_COREUI6_EXPORT diaElemFloat : public diaElem
{
protected:
	int decimals;

public:
  ELEM_TYPE_FLOAT min,max;
  diaElemFloat(ELEM_TYPE_FLOAT *intValue,const char *toggleTitle, ELEM_TYPE_FLOAT min, 
               ELEM_TYPE_FLOAT max,const char *tip=NULL, int decimals = 2);
  virtual ~diaElemFloat() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};
/*************************************************/
class diaMenuEntry
{
public:
    uint32_t    val;
    const char *text;
    const char *desc;
};

class ADM_COREUI6_EXPORT diaMenuEntryDynamic : public diaMenuEntry
{
  public:
  public:
    diaMenuEntryDynamic(uint32_t v,const char *t,const char *d) 
      {
          val=v;
          text=ADM_strdup(t);
          desc=ADM_strdup(d);
      }
    ~diaMenuEntryDynamic() 
      { 
          ADM_dealloc(text);
          ADM_dealloc(desc);
      }
    
};
//*******************************
// static (i.e. hardcoded) menu
//*******************************



//*******************************
// Same but for dynamic menus
//*******************************
typedef diaElem  *(CREATE_MENU_T)(uint32_t *intValue,const char *itle, uint32_t nb, 
        const diaMenuEntry *menu,const char *tip);
typedef diaElem  *(CREATE_MENUDYNAMIC_T)(uint32_t *intValue,const char *itle, uint32_t nb, 
         diaMenuEntryDynamic **menu,const char *tip);

class ADM_COREUI6_EXPORT diaElemMenuDynamicBase : public diaElem
{
protected:	
diaMenuEntryDynamic **menu;
uint32_t            nbMenu;
dialElemLink        links[MENU_MAX_lINK];
uint32_t            nbLink;

public:
	diaElemMenuDynamicBase() : diaElem(ELEM_MENU) {};
  
  virtual   ~diaElemMenuDynamicBase() {};
  virtual uint8_t   link(diaMenuEntryDynamic *entry,uint32_t onoff,diaElem *w)=0;
};

class ADM_COREUI6_EXPORT diaElemMenuDynamic : public diaElemMenuDynamicBase
{
public:
  diaElemMenuDynamic(uint32_t *intValue,const char *itle, uint32_t nb, 
               diaMenuEntryDynamic **menu,const char *tip=NULL);
  
  virtual   ~diaElemMenuDynamic() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  virtual uint8_t   link(diaMenuEntryDynamic *entry,uint32_t onoff,diaElem *w);
  virtual void      updateMe(void);
  virtual void      enable(uint32_t onoff) ;
  virtual void      finalize(void);
  int getRequiredLayout(void);
};

class ADM_COREUI6_EXPORT diaElemMenuBase : public diaElem
{
protected:
	const diaMenuEntry  *menu;
	uint32_t            nbMenu;
	dialElemLink        links[MENU_MAX_lINK];
	uint32_t            nbLink;
public:	
	diaElemMenuBase(void) : diaElem(ELEM_MENU) {};
	virtual ~diaElemMenuBase(void) {};
	virtual uint8_t   link(diaMenuEntry *entry,uint32_t onoff,diaElem *w)=0;
};
class ADM_COREUI6_EXPORT diaElemMenu : public diaElemMenuBase
{

diaElemMenuDynamic  *dyna;
diaMenuEntryDynamic  **menus;
public:
  diaElemMenu(uint32_t *intValue,const char *itle, uint32_t nb, 
               const diaMenuEntry *menu,const char *tip=NULL);
  
  virtual ~diaElemMenu() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  virtual uint8_t   link(diaMenuEntry *entry,uint32_t onoff,diaElem *w);
  virtual void      updateMe(void);
  void      enable(uint32_t onoff) ;
  void      finalize(void);;
  int getRequiredLayout(void);
};

/*************************************************/
#ifndef ADM_MINIMAL_UI_INTERFACE
#include "ADM_encoderConf.h"
typedef diaElem  *(CREATE_BITRATE_T)(COMPRES_PARAMS *p,const char *toggleTitle,const char *tip);
class ADM_COREUI6_EXPORT diaElemBitrateBase : public diaElem
{
  protected:
    COMPRES_PARAMS    copy;
    uint32_t maxQ, minQ;
public:
  
	diaElemBitrateBase(void) : diaElem(ELEM_BITRATE) {};
  virtual ~diaElemBitrateBase() {} ;
  virtual void setMaxQz(uint32_t qz)=0;
};
class ADM_COREUI6_EXPORT diaElemBitrate : public diaElemBitrateBase
{
  protected:
public:
  
  diaElemBitrate(COMPRES_PARAMS *p,const char *toggleTitle,const char *tip=NULL);
  virtual ~diaElemBitrate() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void setMaxQz(uint32_t qz);
  void setMinQz(uint32_t qz);
  
  void updateMe(void);
  int getRequiredLayout(void);
};
#endif
/*************************************************/
typedef diaElem *CREATE_FILE_T(uint32_t writeMode,std::string  &filename,const char *toggleTitle,  const char *defaultSuffix ,const char *tip);
class ADM_COREUI6_EXPORT diaElemFileBase : public diaElem
{

protected:
    const char * defaultSuffix;
public:
  
	diaElemFileBase(void) : diaElem(ELEM_FILE_READ){};
  virtual ~diaElemFileBase() {};
  virtual void   changeFile(void)=0;
  uint32_t _write;

};
class ADM_COREUI6_EXPORT diaElemFile : public diaElemFileBase
{

protected:
    
public:
  
  diaElemFile(uint32_t writeMode,std::string &filename,const char *toggleTitle,
              const char *defaultSuffix = 0,const char *tip=NULL);
  virtual ~diaElemFile() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  
  void   changeFile(void);
  void   enable(uint32_t onoff);
  int getRequiredLayout(void);
};
/*************************************************/
typedef diaElem *CREATE_DIR_T(std::string &filename,const char *toggleTitle,const char *tip);
class ADM_COREUI6_EXPORT diaElemDirSelectBase : public diaElem
{

public:
  
	diaElemDirSelectBase(void) :diaElem(ELEM_DIR_SELECT) {};
  virtual ~diaElemDirSelectBase() {} ;
  virtual void changeFile(void)=0;
};
class ADM_COREUI6_EXPORT diaElemDirSelect : public diaElemDirSelectBase
{

public:
  
  diaElemDirSelect(std::string &filename,const char *toggleTitle,const char *tip=NULL);
  virtual ~diaElemDirSelect() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  
  void changeFile(void);
  void   enable(uint32_t onoff);
  int getRequiredLayout(void);
};
/*************************************************/
/* The text MUST be copied internally ! */
typedef diaElem *(CREATE_READONLYTEXT_T )(const char *readOnly,const char *toggleTitle, const char *tip);
class ADM_COREUI6_EXPORT diaElemReadOnlyText : public diaElem
{

public:
  
  diaElemReadOnlyText(const char *readyOnly,const char *toggleTitle,const char *tip=NULL);
  virtual ~diaElemReadOnlyText() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};
/*************************************************/
typedef diaElem *(CREATE_TEXT_T )(char **readOnly,const char *toggleTitle, const char *tip);
/* The text MUST be copied internally ! */
class ADM_COREUI6_EXPORT diaElemText : public diaElem
{

public:
  
  diaElemText(char **text,const char *toggleTitle,const char *tip=NULL);
  virtual ~diaElemText() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void enable(uint32_t onoff);
  int getRequiredLayout(void);
};

/*********************************************/
typedef diaElem *(CREATE_NOTCH_T )(uint32_t yes,const char *toggleTitle, const char *tip);
class ADM_COREUI6_EXPORT diaElemNotch : public diaElem
{
  uint32_t yesno;
public:
  
  diaElemNotch(uint32_t yes,const char *toggleTitle, const char *tip=NULL);
  virtual ~diaElemNotch() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) ;
  int getRequiredLayout(void);
};
/*********************************************/
typedef diaElem *(CREATE_TAB_T )(const char *toggleTitle, uint32_t nb, diaElem **content);
class ADM_COREUI6_EXPORT diaElemTabs 
{
  public:
    
  uint32_t nbElems;
  diaElem  **dias;
  const char *title;
  
  diaElemTabs(const char *toggleTitle, uint32_t nb, diaElem **content)
  {
      nbElems=nb;
      dias=content; 
      title=toggleTitle;
  }
  virtual ~diaElemTabs() 
  {
  }
};
/**********************************************/
#define DIA_MAX_FRAME 20
class ADM_COREUI6_EXPORT diaElemFrameBase :public diaElem
{
protected:
  uint32_t frameSize;
  uint32_t nbElems;
  diaElem  *elems[DIA_MAX_FRAME];
public:
  
	diaElemFrameBase(void) : diaElem(ELEM_FRAME) {};
  virtual ~diaElemFrameBase() {};
  virtual void swallow(diaElem *widget)=0;
};
typedef diaElem *(CREATE_FRAME_T )(const char *toggleTitle, const char *tip);
class ADM_COREUI6_EXPORT diaElemFrame : public diaElemFrameBase
{
  
public:
  
  diaElemFrame(const char *toggleTitle, const char *tip=NULL);
  virtual ~diaElemFrame() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) ;
  void swallow(diaElem *widget);
  void enable(uint32_t onoff);
  void finalize(void);
  int getRequiredLayout(void);
};
/**********************************************/
typedef diaElem *(CREATE_HEX_T )(const char *toggleTitle, uint32_t dataSize,uint8_t *data);
class ADM_COREUI6_EXPORT diaElemHex : public diaElem
{
  uint32_t dataSize;
  uint8_t  *data;
  
public:
  
  diaElemHex(const char *toggleTitle, uint32_t dataSize,uint8_t *data);
  virtual ~diaElemHex() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) ;
  void finalize(void);
  int getRequiredLayout(void);
};
/**********************************************/
typedef diaElem *(CREATE_THREADCOUNT_T)(uint32_t *value, const char *title, const char *tip );
class ADM_COREUI6_EXPORT diaElemThreadCount : public diaElem
{

public:
  
  diaElemThreadCount(uint32_t *value, const char *title, const char *tip = NULL);
  virtual ~diaElemThreadCount() ;
  void setMe(void *dialog, void *opaque, uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};
/**********************************************/
typedef diaElem *(CREATE_ASPECTRATIO_T)(uint32_t *num, uint32_t *den, const char *title, const char *tip );
class ADM_COREUI6_EXPORT diaElemAspectRatio : public diaElem
{
public:
	uint32_t *den;
	void *denControl, *label;

	diaElemAspectRatio(uint32_t *num, uint32_t *den, const char *title, const char *tip = NULL);
	virtual ~diaElemAspectRatio();
	void setMe(void *dialog, void *opaque, uint32_t line);
	void getMe(void);
	void enable(uint32_t onoff);
	int getRequiredLayout(void);
        void finalize(void);
};

/*********************************************/
ADM_COREUI6_EXPORT uint8_t diaFactoryRun(const char *title,uint32_t nb,diaElem **elems);
ADM_COREUI6_EXPORT uint8_t diaFactoryRunTabs(const char *title,uint32_t nb,diaElemTabs **tabs);
ADM_COREUI6_EXPORT void *  diaFactoryRunTabsPrepare(const char *title,uint32_t nb,diaElemTabs **tabs);
ADM_COREUI6_EXPORT bool    diaFactoryRunTabsFinish(void *f);


/*********************************************/
