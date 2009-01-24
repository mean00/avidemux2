/***************************************************************************
      UI independant part of the OCR engine
      (C) 2007 Mean
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
#include "ADM_default.h"

 #include "ADM_editor/ADM_edit.hxx"
 #include "ADM_videoFilter.h"
 #include "ADM_ocr.h"
 #include "ADM_ocrInternal.h"
 #include "DIA_coreToolkit.h"

#include "ADM_video/ADM_vidMisc.h"

/**************************
 *  UI Dependant part
 *************************/
uint8_t ADM_ocrUpdateNbLines(void *ui,uint32_t cur,uint32_t total);
uint8_t ADM_ocrUpdateNbGlyphs(void *ui,uint32_t nbGlyphs);
uint8_t ADM_ocrUpdateTextAndTime(void *ui,char *decodedString,char *timeCode);
uint8_t ADM_ocrDrawFull(void *d,uint8_t *data);
uint8_t ADM_ocrUiEnd(void *d);
void 	*ADM_ocrUiSetup(void);
uint8_t ADM_ocrSetRedrawSize(void *ui,uint32_t w,uint32_t h);

extern void UI_purge(void);
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++
        Main
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/**
 * 		\fn 		ADM_ocr_engine
 * 		\brief 		Common part of the OCR engine
 */
uint8_t ADM_ocr_engine(ADM_OCR_SOURCE & source,const char *labelSrt,admGlyph *head)
{
    uint32_t nbSub=0;
    FILE *out=NULL;
    vobSubBitmap *bitmap=NULL;
    ADM_BitmapSource *bitmapSource=NULL;
    uint32_t startTime,endTime;
    uint32_t w,h,oldw=0,oldh=0;
    uint32_t oldbitmapw=0;
    uint32_t oldbitmaph=0;
    uint32_t first,last;
    uint32_t seqNum;
    char     text[1024];
    ReplyType reply;
    void *ui=NULL;
    char decodedString[1024];
    uint8_t *workArea=NULL;
    uint32_t nbGlyphs=0;
    
    ui=ADM_ocrUiSetup();
    
    int success = 1;

_again:    
    
    UI_purge();
 
    out=fopen(labelSrt,"wb");
    if(!out)
    {
        success = 0;
       GUI_Error_HIG(QT_TR_NOOP("Output file error"), QT_TR_NOOP("Could not open \"%s\" for writing."), labelSrt);
       goto endIt;
    }

    bitmapSource=ADM_buildBitmapSource(&source);
    if(!bitmapSource)
    {
        success = 0;
        GUI_Error_HIG(QT_TR_NOOP("Input file error"), QT_TR_NOOP("Unknown source type"));
        goto endIt;
    }
    if(!bitmapSource->init(&source))
    {
        success = 0;
        GUI_Error_HIG(QT_TR_NOOP("Input file error"), QT_TR_NOOP("Bitmap source failed"));
        goto endIt;
    }
    
    nbSub=bitmapSource->getNbImages();
    if(!nbSub)
    {
        success = 0;
        GUI_Error_HIG(QT_TR_NOOP("Problem loading sub"),QT_TR_NOOP("No subs"));
        goto endIt;
     }

    seqNum=1;   // Sub number in srt file
    oldw=oldh=0;
    uint32_t eos;
    //******************    
    // Load all bitmaps
    //******************
    for(uint32_t i=0;i<nbSub;i++)
    {
            first=last=0;
            bitmap=bitmapSource->getBitmap(i,&startTime, &endTime,&first,&last,&eos);
            if(eos) break;
            ADM_assert(last>=first);
            
            if(!bitmap) continue;
            if(first==last) continue;

            // If the bitmap size changed or does not exist yet...
            if(!workArea || oldbitmapw!=bitmap->_width || oldbitmaph!=bitmap->_height)
            {
              if(workArea) 
              {
                delete [] workArea;
                workArea=NULL; 
              }
              // Workarea is actually bigger than what we use
              workArea=new uint8_t[bitmap->_width*(bitmap->_height)];
              memset(workArea,0,bitmap->_width*(bitmap->_height));
            }
            oldbitmaph=bitmap->_height;
            oldbitmapw=bitmap->_width;

           w=bitmap->_width;
           h=last-first+1;
           
           ADM_ocrSetRedrawSize(ui,w,h);
           
           // Build
againPlease:
           mergeBitmap(bitmap->_bitmap+first*w, workArea, bitmap->_alphaMask+first*w,  w,   h);
           if(oldw!=w || oldh !=h)
           {                
                UI_purge();  // Force redaw
           }
           // Merge & draw
             ADM_ocrDrawFull(ui,workArea);
             // OCR
              reply=ocrBitmap(workArea,w,h,decodedString,head);
              if(reply==ReplyClose) goto endIt;
              if(reply==ReplyCalibrate)
                {
                        ocrUpdateMinThreshold();
                        goto againPlease;
                }
             
             fprintf(out,"%d\n",seqNum++);
             uint16_t hh,mm,ss,ms;
             ms2time(startTime, &hh, &mm, &ss, &ms);
             fprintf(out,"%02d:%02d:%02d,%03d --> ",hh,mm,ss,ms);
             
             // Update Timecode
             char timeCode[100];
             snprintf(timeCode,100,"%02d:%02d:%02d,%03d",hh,mm,ss,ms);
             ADM_ocrUpdateTextAndTime(ui,decodedString,timeCode);
             
             ms2time(endTime, &hh, &mm, &ss, &ms);
             fprintf(out,"%02d:%02d:%02d,%03d\n",hh,mm,ss,ms);
             fprintf(out,"%s\n\n",decodedString);

             oldw=w;
             oldh=h;
             // Update infos
             ADM_ocrUpdateNbLines(ui,i+1,nbSub);
             ADM_ocrUpdateNbGlyphs(ui,nbGlyphs);
    }

endIt:
	ADM_ocrUiEnd(ui);
	ui=NULL;
    
    if(out) 
          fclose(out);
      out=NULL;
      
      if(bitmapSource)
    	        delete bitmapSource;
    	    bitmapSource=NULL;
      
    return success;
}

//EOF
