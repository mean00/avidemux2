/***************************************************************************
                          DIA_ocr
                             -------------------
                        Ui for OCR

    begin                : 08 Apr 2005
    copyright            : (C) 2004/5 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <math.h>

#include "ADM_default.h"

#include "DIA_factory.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"


#include "ADM_videoFilter/ADM_vobsubinfo.h"
#include "ADM_videoFilter/ADM_vidVobSub.h"
#include "ADM_ocr/ADM_ocr.h"
#include "DIA_coreToolkit.h"
#include "prefs.h"


#include "ADM_ocr/ADM_ocr.h"

extern uint8_t DIA_vobsub(vobSubParam *param);

static void cb_idx(void *foo);
static void cleanupSub(ADM_OCR_SOURCE *p);
/**
    \fn DIA_ocrGen
    \brief Dialog to select input & output files before calling the actual ocr engine
*/
uint8_t DIA_ocrGen(void)
{

  vobSubParam subparam={NULL,0,0};
  char *srtFileName=NULL;
  char *glyphFileName=NULL;
  admGlyph head(16,16);
  char *globalGlyph=NULL;
  uint32_t globalGlyphOn=0;
  ADM_OCR_SOURCE source;
  
  memset(&source,0,sizeof(source));
  
  source.type=ADM_OCR_TYPE_VOBSUB;
  source.subparam=&subparam;
  
  prefs->get(FEATURE_GLOBAL_GLYPH_ACTIVE,&globalGlyphOn);
  if(globalGlyphOn)
  {
     prefs->get(FEATURE_GLOBAL_GLYPH_NAME,&globalGlyph);
     if(!*globalGlyph)
     {
        ADM_dezalloc(globalGlyph);
        globalGlyph=NULL; 
     }
  }

  if(globalGlyph)
  {
    glyphFileName=globalGlyph;
  }
_again:  
  // Fist build a dialogFactory to get input and output files
  diaElemButton   selectIdx(QT_TR_NOOP("Select idx file:"), cb_idx,&subparam,NULL);
  diaElemFile     selectGlyph(1,&glyphFileName,QT_TR_NOOP("Use GlyphSet (optional):"), NULL, QT_TR_NOOP("Select GlyphSet file"));
  diaElemFile     selectSrt(1,&srtFileName,QT_TR_NOOP("Output SRT file"), NULL, QT_TR_NOOP("Save SRT file"));
  
  diaElem *elems[]={&selectIdx,&selectSrt,&selectGlyph};
  
   uint32_t n=3;
   if(globalGlyph)
   {
     n--; // Remove glyph from dialog
   }
  
    if(!diaFactoryRun(QT_TR_NOOP("Select input and ouput files"),n,elems))
        {
          cleanupSub(&source);
          if(srtFileName )ADM_dezalloc(srtFileName);
          srtFileName=NULL;
          destroyGlyphTree(&head);
          return 0;
        }
        if(!ADM_fileExist(subparam.subname))
        {
          GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP("The idx/sub file does not exist."));
          goto _again; 
        }
        if(!srtFileName || !*srtFileName)
        {
          GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP("Please Select a valid output SRT file."));
          goto _again; 
        }
         if(glyphFileName && *glyphFileName)
         {
           if(!ADM_fileExist(glyphFileName))
            {
              GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP("The idx/sub file does not exist."));
              goto _again; 
            }
            // Purge previous glyph set if any
            destroyGlyphTree(&head);
            uint32_t nb;
            printf("[OCR] Loading glyphset :<%s>\n",glyphFileName);
            if(!loadGlyph(glyphFileName,&head,&nb))
            {
              GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP("Cannot load the glyphset file."));
              goto _again;               
            }
            printf("[GLYPH] Found %u glyph\n");
         }
        // We have our SRT and our idx/sub files : Go go go
         
    if(ADM_ocr_engine(source,srtFileName,&head))
    {
        // Save glyph set 
        if(globalGlyph)
        {
          uint32_t nb=1;
           saveGlyph(globalGlyph,&head,nb);
        }
        else
        {
            char *save=NULL;
            uint32_t nb=1;
              diaElemFile     selectSave(1,&save,QT_TR_NOOP("GlyphSet filename"), NULL, QT_TR_NOOP("Save GlyphSet file"));
              diaElem *elems2[]={&selectSave};
            if(diaFactoryRun(QT_TR_NOOP("Save Glyph"),1,elems2))
            {
              saveGlyph(save,&head,nb);
            }
            if(save) ADM_dezalloc(save);
        }
    }

  cleanupSub(&source);
  if(srtFileName )ADM_dezalloc(srtFileName);
  srtFileName=NULL;
  destroyGlyphTree(&head);
  return 1;  
}
/**
    \fn DIA_ocrDvb
    \brief Dialog to select input & output files before calling the actual ocr engine
*/
uint8_t DIA_ocrDvb(void)
{

  vobSubParam subparam={NULL,0,0};
  char *srtFileName=NULL;
  char *glyphFileName=NULL;
  char *tsFileName=NULL;
  admGlyph head(16,16);
  char *globalGlyph=NULL;
  uint32_t globalGlyphOn=0;
  uint32_t pid=0x96;
  ADM_OCR_SOURCE source;
  
  memset(&source,0,sizeof(source));
  source.type=ADM_OCR_TYPE_TS;
  
  prefs->get(FEATURE_GLOBAL_GLYPH_ACTIVE,&globalGlyphOn);
  if(globalGlyphOn)
  {
     prefs->get(FEATURE_GLOBAL_GLYPH_NAME,&globalGlyph);
     if(!*globalGlyph)
     {
        ADM_dezalloc(globalGlyph);
        globalGlyph=NULL; 
     }
  }

  if(globalGlyph)
  {
    glyphFileName=globalGlyph;
  }
_againX:  
  // Fist build a dialogFactory to get input and output files
  diaElemFile     selectTs(1,&tsFileName,QT_TR_NOOP("Input TS:"), NULL, QT_TR_NOOP("Select TS file"));
  diaElemUInteger selectPid(&pid,QT_TR_NOOP("Subtitle PID:"),0,255);
  diaElemFile     selectGlyph(1,&glyphFileName,QT_TR_NOOP("Use glyphset (optional):"), NULL, QT_TR_NOOP("Select GlyphSet file"));  
  diaElemFile     selectSrt(1,&srtFileName,QT_TR_NOOP("Output SRT file"), NULL, QT_TR_NOOP("Save SRT file"));
  
  diaElem *elems[]={&selectTs,&selectPid,&selectSrt,&selectGlyph};
  
  
   uint32_t n=4;
   if(globalGlyph)
   {
     n--; // Remove glyph from dialog
   }
  
        if( !diaFactoryRun(QT_TR_NOOP("Select input and ouput files"),n,elems))
        {
          cleanupSub(&source);
          if(srtFileName )ADM_dezalloc(srtFileName);
          srtFileName=NULL;
          destroyGlyphTree(&head);
          return 0;
        }
        // TS file exists ?
        if(!ADM_fileExist(tsFileName))
        {
        	  GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP("Please Select a valid TS file."));
        	  goto _againX;
        }
       
        if(!srtFileName || !*srtFileName)
        {
          GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP("Please Select a valid output SRT file."));
          goto _againX; 
        }
         if(glyphFileName && *glyphFileName)
         {
           if(!ADM_fileExist(glyphFileName))
            {
              GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP("The idx/sub file does not exist."));
              goto _againX; 
            }
            // Purge previous glyph set if any
            destroyGlyphTree(&head);
            uint32_t nb;
            printf("[OCR] Loading glyphset :<%s>\n",glyphFileName);
            if(!loadGlyph(glyphFileName,&head,&nb))
            {
              GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP("Cannot load the glyphset file."));
              goto _againX;               
            }
            printf("[GLYPH] Found %u glyph\n");
         }
        // We have our SRT and our TS file
        // Call the OCR engine...
         source.TsFile=ADM_strdup(tsFileName);
         source.TsPid=pid;
         ADM_ocr_engine(source,srtFileName,&head);
        
        // Save glyph set 

        if(globalGlyph)
        {
          uint32_t nb=1;
           saveGlyph(globalGlyph,&head,nb);
        }else
        {
            char *save=NULL;
            uint32_t nb=1;
              diaElemFile     selectSave(1,&save,QT_TR_NOOP("GlyphSet filename"), NULL, QT_TR_NOOP("Save GlyphSet file"));
              diaElem *elems2[]={&selectSave};
            if( diaFactoryRun(QT_TR_NOOP("Save GlyphSet"),1,elems2))
            {
              saveGlyph(save,&head,nb);
            }
            if(save) ADM_dezalloc(save);
        }

  cleanupSub(&source);
  if(srtFileName )ADM_dezalloc(srtFileName);
  srtFileName=NULL;
  
  if(tsFileName )ADM_dezalloc(tsFileName);
  tsFileName=NULL;
  
  destroyGlyphTree(&head);
  return 1;  
}
/**
		\fn 	cleanupSub
		\brief 	Free all ressources allocated to source
*/
void cleanupSub(ADM_OCR_SOURCE *p)
{
  if(p->TsFile)
  {
	  	ADM_dezalloc(p->TsFile);
	  	p->TsFile=NULL;
  }
  if(p->subparam)
  {
	  vobSubParam *subparam=p->subparam;
	  if(subparam->subname)
	  {
		  ADM_dezalloc(subparam->subname);
		  subparam->subname=NULL;
	  }
	  
  }
  
}
/**
	\fn cb_idx
	\brief Callback to select sub/language/...
*/
void cb_idx(void *foo)
{
   vobSubParam *bar=(vobSubParam *)foo;
    DIA_vobsub(bar);
}
//EOF
