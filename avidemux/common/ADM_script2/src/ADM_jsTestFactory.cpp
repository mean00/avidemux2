/**
    \file ADM_JSif.cpp
    \brief interface to js

 Author: Anish Mistry/mean/gruntster
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_js.h"
#include "ADM_editor/ADM_edit.hxx"
#include "DIA_fileSel.h"
#include "DIA_factory.h"


extern "C"
{
    void jsCrashTest(void);
    void jsAssertTest(void);
}

/**
    \fn crashTest
    \brief Force a crash
*/
void jsCrashTest(void)
{
  
  int *foobar=NULL;
  *foobar=0; // CRASH!

}
/**
    \fn assertTest
    \brief Force a crash
*/
void jsAssertTest(void)
{
  
  ADM_assert(0);

}


/**
    \fn jsFacInt
*/
bool jsFacInt(void)
{
  uint32_t tog=0;
   diaElemUInteger blend(&tog,QT_TR_NOOP("Uinteger"),0,255);
    diaElem *elems[]={&blend   };
    
  if(diaFactoryRun(QT_TR_NOOP("Test uinteger"),1,elems))
  {
    jsLog(JS_LOG_NORMAL,"Value : %u\n",tog);
    return true;
  }
  return false;
}

/**
    \fn jsFacFloat
*/
bool jsFacFloat(void)
{
  ELEM_TYPE_FLOAT tog=0;
   diaElemFloat blend(&tog,QT_TR_NOOP("Float"),0,255);
    diaElem *elems[]={&blend   };
    
  if(diaFactoryRun("Test float",1,elems))
  {
    jsLog(JS_LOG_NORMAL,"Value : %f\n",(float)tog);
    return true;
  }
  return false;
}

/**
    \fn jsFacToggle
*/
bool jsFacToggle(void)
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
    jsLog(JS_LOG_NORMAL,"Value : %u\n",tog);
    return true;
  }
  return false;
}

/**
    \fn jsFacMenu
*/
bool jsFacMenu(void)
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
    jsLog(JS_LOG_NORMAL,"Value : %"LU"\n",tog);
    return true;
  }
  return false;
}

/**
    \fn jsFacFile
*/
bool jsFacFile(void)
{
   uint32_t tog=0;
   char *test=ADM_strdup("Entry test1");
    
      diaElemFile fread(0,&test,"Entry");
      diaElem *elems[]={&fread   };
  if(diaFactoryRun("Test FileRead",1,elems))
   {
    jsLog(JS_LOG_NORMAL,"Value : %s\n",test);
    if(test) ADM_dealloc(test);
    return true;
  }
 if(test) ADM_dealloc(test);
  return false;
}

/**
    \fn jsFacDirSel
*/
bool jsFacDirSel(void)
{
   uint32_t tog=0;
   char *test=ADM_strdup("Entry test1");
    
  diaElemDirSelect fread(&test,"Entry");
  diaElem *elems[]={&fread   };
  if(diaFactoryRun("Test DirSel",1,elems))
  {
    jsLog(JS_LOG_NORMAL,"Value : %s\n",test);
    if(test) ADM_dealloc(test);
    return true;
  }
 if(test) ADM_dealloc(test);
  return false;
}

/**
    \fn jsFacBitrate
*/
bool jsFacBitrate(void)
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
    
    return true;
  }
  return false;
}

/**
    \fn jsFacInt
*/
bool jsFacBar(void)
{
    
      diaElemBar bar1(25,"25");
      diaElemBar bar2(65,"65");
      diaElem *elems[]={&bar1,&bar2   };
  if(diaFactoryRun("Test FileRead",2,elems))
  {
   return true;
  }
  return false;
}

void clickMe(void *cookie)
{
  GUI_Error_HIG("Button","Button pressed!"); 
}

/**
    \fn jsFacButton
*/
bool jsFacButton(void)
{
    
      diaElemButton bar1("Button",clickMe,NULL);
      diaElem *elems[]={&bar1   };
  if(diaFactoryRun("Test Button",1,elems))
  {
    return true;
  }
  return false;
}

/**
    \fn jsFacSlider
*/
bool jsFacSlider(void)
{
  int32_t val=4;
      diaElemSlider slide(&val,"foo", 0,10);
      
      diaElem *elems[]={&slide   };
  if(diaFactoryRun("Test Slider",1,elems))
  {
    jsLog(JS_LOG_NORMAL,"Value : %d\n",(int)val);
    return true;
  }
  return false;
}

/**
    \fn jsFacInt
*/
bool jsFacRoText(void)

{
    
      diaElemReadOnlyText txt("blah blah","Value:");
      
      diaElem *elems[]={&txt   };
  if(diaFactoryRun("Test FileRead",1,elems))
  {
    return true;
  }
  return false;
}

/**
    \fn jsFacText
*/
bool jsFacText(void)
{
    
      char *foo=ADM_strdup("blah");
      diaElemText txt(&foo,"Text",NULL);
      
      diaElem *elems[]={&txt   };
  if(diaFactoryRun("Test FileRead",1,elems))
 {
    jsLog(JS_LOG_NORMAL,"Out:%s",foo);
    if(foo) ADM_dealloc(foo);
    return true;
  }
  if(foo) ADM_dealloc(foo);
  return false;
}

/**
    \fn jsFacInt
*/
bool jsFacTab(void)
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
    return true;
  }
  return false;
}

/**
    \fn jsFacInt
*/
bool jsFacFrame(void)
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
    return true;
  }
  return false;
}


/**
    \fn jsFacHex
*/
bool jsFacHex(void)
{
    
      uint8_t data[100];
      for(int i=0;i<100;i++) data[i]=i;
      
      diaElemHex binhex("*****",100,data);
      
      
         diaElem *elems[]={&binhex   };
  if(diaFactoryRun("Test binHex",1,elems))
  {
    return true;
  }
  return false;
      
}

/**
    \fn jsFacMatrix
*/
bool jsFacMatrix(void)
{
    
      uint8_t data[16];
      for(int i=0;i<100;i++) data[i]=i;
      
      diaElemMatrix Matrix(data,"Matrix",4);
      
      
         diaElem *elems[]={&Matrix   };
  if(diaFactoryRun("Test Matrix",1,elems))
  {
      for(int x=0;x<4*4;x++)
      {
          if(x && !(x&3)) printf("\n");
          jsLog(JS_LOG_NORMAL,"%02x ",data[x]);
          
      }
      return true;
  }
  return false;
}

/**
    \fn jsFacThreadcount
*/
bool jsFacThreadcount(void)
{
	uint32_t val=1;
	diaElemThreadCount threadcount(&val,"ThreadCount");
      
    diaElem *elems[]={&threadcount   };
    
  if(diaFactoryRun("Test ThreadCount",1,elems))
  {
         jsLog(JS_LOG_NORMAL,"Thread: %u ",(unsigned int)val);
        return true;
  }
  return false;
}

/**
    \fn jsFacNotch
*/
bool jsFacNotch(void)
{
    
	diaElemNotch notch(1,"Notch");
      
         diaElem *elems[]={&notch   };
  if(diaFactoryRun("Test Notch",1,elems))
  {
    return true;
  }
  return false;
      
}

//EOF 
