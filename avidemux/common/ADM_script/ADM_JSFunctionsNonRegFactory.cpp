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
#include <sys/types.h>
#include <sys/stat.h>
#if defined( ADM_WIN32 ) || defined(__MINGW32__)
#define ADM_JS_WIN32
#endif

#ifndef ADM_JS_WIN32
#include <sys/wait.h>
#include <sys/param.h>
#endif
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <vector>
#include <string>
#include "ADM_JSAvidemux.h"
#include "ADM_JSGlobal.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "ADM_editor/ADM_outputfmt.h"
#include "adm_scanner.h" 
#include "avi_vars.h"
#include "gui_action.hxx"

//#include "ADM_videoFilter.h"
#include "ADM_editor/ADM_outputfmt.h"

#include "ADM_script/ADM_container.h"

#include "ADM_JSGlobal.h"
#include "DIA_fileSel.h"

#include "DIA_factory.h"


extern char **environ;
extern char *script_getVar(char *in, int *r);

JSBool facInt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facFloat(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facToggle(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facMenu(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facBitrate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facBar(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facRoText(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facText(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facTab(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facFrame(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facHex(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facDirSel(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facButton(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facMatrix(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facNotch(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facThreadCount(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facSlider(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool crashTest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool assertTest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool facInt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  uint32_t tog=0;
   diaElemUInteger blend(&tog,QT_TR_NOOP("Uinteger"),0,255);
    diaElem *elems[]={&blend   };
    
  if(diaFactoryRun(QT_TR_NOOP("Test uinteger"),1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    printf("Value : %u\n",tog);
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
}
JSBool facFloat(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  ELEM_TYPE_FLOAT tog=0;
   diaElemFloat blend(&tog,QT_TR_NOOP("Float"),0,255);
    diaElem *elems[]={&blend   };
    
  if(diaFactoryRun("Test float",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    printf("Value : %f\n",(float)tog);
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  
  return JS_TRUE;
}

JSBool facToggle(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  uint32_t tog=0;
  uint32_t test=0;
   diaElemToggle blend(&tog,QT_TR_NOOP("Toggle"));
    diaElemUInteger     bt(&test,"Entry",0,10);
    diaElemUInteger     bt2(&test,"Entry",0,10);
    diaElem *elems[]={&blend,&bt,&bt2   };
    blend.link(1,&bt);
    blend.link(0,&bt2);
    
  if(diaFactoryRun("Test Toggle",3,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    printf("Value : %u\n",tog);
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  return JS_TRUE;
}

JSBool facMenu(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
   uint32_t tog=4;
   ELEM_TYPE_FLOAT f=1; 
   
    diaMenuEntry menu[]={
                             {2,   QT_TR_NOOP("No Strategy"),NULL},
                             {4,     QT_TR_NOOP("3:2 Pulldown"),NULL},
                             {6,     QT_TR_NOOP("Pal/Secam"),NULL},
                             {7,  QT_TR_NOOP("NTSC converted from PAL"),NULL}
                          };
   diaElemMenu blend(&tog,QT_TR_NOOP("menu"),4,menu);
    
    // Link it to another
    diaElemFloat toggle(&f,"Linked float",1,2);
    blend.link(&(menu[1]),1,&toggle);
    //
diaElem *elems[]={&blend,&toggle   };
  if(diaFactoryRun("Test Menu",2,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    printf("Value : %u\n",tog);
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  return JS_TRUE;
}
JSBool facFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
   uint32_t tog=0;
   char *test=ADM_strdup("Entry test1");
    
      diaElemFile fread(0,&test,"Entry");
      diaElem *elems[]={&fread   };
  if(diaFactoryRun("Test FileRead",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    printf("Value : <%s>\n",test);
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  if(test) ADM_dealloc(test);
  return JS_TRUE;
}
JSBool facDirSel(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
   uint32_t tog=0;
   char *test=ADM_strdup("Entry test1");
    
      diaElemDirSelect fread(&test,"Entry");
      diaElem *elems[]={&fread   };
  if(diaFactoryRun("Test DirSel",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    printf("Value : <%s>\n",test);
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  if(test) ADM_dealloc(test);
  return JS_TRUE;
}

JSBool facBitrate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

   COMPRES_PARAMS test={
  COMPRESS_CQ,
  1,
  1500,
  700,
  1000,
  ADM_ENC_CAP_CQ+ADM_ENC_CAP_2PASS+ADM_ENC_CAP_CBR+ADM_ENC_CAP_SAME
  };
    
      diaElemBitrate bt(&test,"Entry");
      diaElem *elems[]={&bt   };
  if(diaFactoryRun("Test BitRate",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  return JS_TRUE;
}

JSBool facBar(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
      diaElemBar bar1(25,"25");
      diaElemBar bar2(65,"65");
      diaElem *elems[]={&bar1,&bar2   };
  if(diaFactoryRun("Test FileRead",2,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
}

void clickMe(void *cookie)
{
  GUI_Error_HIG("Button","Button pressed!"); 
}
JSBool facButton(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
      diaElemButton bar1("Button",clickMe,NULL);
      diaElem *elems[]={&bar1   };
  if(diaFactoryRun("Test Button",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
}
JSBool facSlider(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  int32_t val=4;
      diaElemSlider slide(&val,"foo", 0,10);
      
      diaElem *elems[]={&slide   };
  if(diaFactoryRun("Test Slider",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
}
JSBool facRoText(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
      diaElemReadOnlyText txt("blah blah","Value:");
      
      diaElem *elems[]={&txt   };
  if(diaFactoryRun("Test FileRead",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
}
JSBool facText(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
      char *foo=ADM_strdup("blah");
      diaElemText txt(&foo,"Text",NULL);
      
      diaElem *elems[]={&txt   };
  if(diaFactoryRun("Test FileRead",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
}

JSBool facTab(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
      uint32_t test,test2;
      
      diaElemReadOnlyText txt("blah blah","Value:");
      diaElemUInteger     bt(&test,"Entry",0,10);
      diaElemUInteger     bt2(&test2,"Entry",0,10);
      
      
      diaElem *elems1[]={&txt   };
      diaElem *elems2[]={&bt,&bt2   };
      
      diaElemTabs tab1("T1",1,(diaElem **)elems1);
      diaElemTabs tab2("T2",2,(diaElem **)elems2);
      
      diaElemTabs *tabs[2]={&tab1,&tab2};
          
      
  if(diaFactoryRunTabs("Test FileRead",2,tabs))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
}
JSBool facFrame(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
      uint32_t test,test2;
      
      diaElemReadOnlyText align("*****","Value:");
      diaElemReadOnlyText txt("blah blah","Value:");
      diaElemUInteger     bt(&test,"Entry1",0,10);
      diaElemUInteger     bt2(&test2,"Entry2",0,10);
      diaElemFrame        frm("Frame1");
      
      frm.swallow(&txt);
      frm.swallow(&bt);
      frm.swallow(&bt2);
      
         diaElem *elems[]={&align,&frm   };
  if(diaFactoryRun("Test frame",2,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
      
      
}


JSBool facHex(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
      uint8_t data[100];
      for(int i=0;i<100;i++) data[i]=i;
      
      diaElemHex binhex("*****",100,data);
      
      
         diaElem *elems[]={&binhex   };
  if(diaFactoryRun("Test binHex",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  return JS_TRUE;
      
      
}
JSBool facMatrix(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
      uint8_t data[16];
      for(int i=0;i<100;i++) data[i]=i;
      
      diaElemMatrix Matrix(data,"Matrix",4);
      
      
         diaElem *elems[]={&Matrix   };
  if(diaFactoryRun("Test Matrix",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  
  for(int x=0;x<4*4;x++)
  {
	  if(x && !(x&3)) printf("\n");
	  printf("%02x ",data[x]);
	  
  }
  
  return JS_TRUE;
      
      
}
JSBool facThreadCount(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	uint32_t val=1;
	diaElemThreadCount threadcount(&val,"ThreadCount");
      
    diaElem *elems[]={&threadcount   };
    
  if(diaFactoryRun("Test ThreadCount",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  return JS_TRUE;
      
}
JSBool facNotch(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    
	diaElemNotch notch(1,"Notch");
      
         diaElem *elems[]={&notch   };
  if(diaFactoryRun("Test Notch",1,elems))
  {
    *rval = BOOLEAN_TO_JSVAL(1);
    
  }else
    *rval = BOOLEAN_TO_JSVAL(0);
  return JS_TRUE;
      
      
}

//EOF 
