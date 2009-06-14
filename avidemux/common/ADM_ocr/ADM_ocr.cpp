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
  #include "DIA_enter.h"
static uint32_t minThreshold=0x80;

/* In the UI related code */
extern void UI_purge(void);

extern uint8_t adm_estimate_glyphSize(admGlyph *glyph,uint32_t *minx, uint32_t *maxx,uint32_t *miny,uint32_t *maxy,int *raw);

static uint32_t minAlpha=7; /* Below minAlpha is is considered black */

/**
      \fn mergeBitmap
      \brief Merge bitmap with alpha mask so that we go a black & white output
      @param bitin bitmap (input)
      @param bitout B&W bitmap (output)
      @param maskin alpha mask (input)
      @param w width of bitmap
      @param h height of bitmap

*/
uint8_t mergeBitmap(uint8_t *bitin, uint8_t *bitout, uint8_t *maskin,uint32_t w, uint32_t h)
{
// Merge with alpha channel
           uint8_t *in,*mask,*out;
           uint32_t alp,nw;
           in=bitin;
           out=bitout;
           mask=maskin;
           for(uint32_t y=0;y<h;y++)
           {
            for(uint32_t x=0;x<w;x++)
            {
                   
                    nw=in[x];
                    alp=mask[x];

                    if(alp>minAlpha&& nw >minThreshold)  nw=0xff;
                         else       nw=0;
                        
                    out[x]=nw;
            }
            out+=w;
            in+=w;
            mask+=w;
           }    
    return 1;
}
/**
      \fn ocrBitmap
      \brief Split the bitmap into glyphes, ocr glyphes and output text
      @param workArea, Bitmap to work with
      @param w width of bitmap
      @param h height of bitmap
      @param decodedString Will contain ocr'ed text
*/
ReplyType ocrBitmap(uint8_t *workArea,uint32_t w,uint32_t h,char *decodedString,admGlyph *head)
{
uint8_t found;
uint32_t colstart=0,colend=0,oldcol;
uint32_t line=0,nbLine=1;
uint32_t base=0,bottom,top;    
ReplyType reply;
    // Search First non nul colum
    decodedString[0]=0;
    // Search how much lines there is in the file
    //
    top=bottom=0;
    while(top<h)
    {
        // Search non empty line as top
        while(top<h && lineEmpty(workArea,w,w,top)) top++;
        // Nothing found
        if(top>=h-1) break;

        // 
       

        bottom=top+1;
        // Search empty line if any, bottom is the 1st line full of zero
        while(bottom<h && (!lineEmpty(workArea,w,w,bottom) || bottom-top<7))
        {
            bottom++;
        }
        if(line) strcat(decodedString,"\n"); 
        //printf("\n Top:%lu bottom:%lu\n",top,bottom);
       
        // Scan a full line
        colstart=0;
        oldcol=0;
       
        // Split a line into glyphs
        while(colstart<w)
        {
            UI_purge();
            oldcol=colstart;
            while( columnEmpty(workArea+colstart+top*w, w, bottom-top) && colstart<w) colstart++;
            if(colstart>=w) break;
            // if too far apart, it means probably a blank space
            if(colstart-oldcol>6)
            {
                strcat(decodedString," ");
            }
       
            // We have found a non null column
            // Seek the end now
            colend=colstart+1;
            while( !columnEmpty(workArea+colend+top*w, w, bottom-top) && colend<w) colend++;
         
         
            // printf("Found glyph: %lu %lu\n",colstart,colend);  
            reply=handleGlyph(workArea,colstart,colend,w,bottom,top,head,decodedString);
            switch(reply)
                {
                        case ReplySkip:break;
                        case ReplyOk:break;
                        case ReplyClose:
                        case ReplyCalibrate: return reply;break;
            
                        case ReplySkipAll: return ReplyOk;break;
                        default: ADM_assert(0);
                }
            
            
            colstart=colend;
      }
      line++;      
      top=bottom;
      
    }
   
    return ReplyOk;
}

/**
      \fn handleGlyph
      \brief Handle ONE glyph
      @param workArea full bitmap to OCR 
      @param start Start column of glyph
      @param end end column of glyph
      @param w Width of bitmap
      @param h Height of bitmap
      @param base Baseline of glyph
    We now have a good candidate for the glyph.
    We will do the following processing :
        - Clip the glyph to have it in its bounding box
        - extract its container. If the container is smaller than the glyph, it means
                that we have in fact several glyphs that overlaps slightly. In
                that case we use another method to extract the glyph.
                We split it using leftturn method and do it again.
*/
ReplyType handleGlyph(uint8_t *workArea,uint32_t start, uint32_t end,uint32_t w,uint32_t h,uint32_t base,
							admGlyph *head,char *decodedString)
{
uint8_t found=0;
static int inc=1;
ReplyType reply;
          
    
    // Ok now we have the cropped glyp
    
    admGlyph *glyph,*cand;
    uint32_t minx,maxx,miny,maxy;
    int     *raw=NULL;
            glyph=new admGlyph(end-start,h-base);
            glyph->create(workArea+start+base*w,w);
            glyph=clippedGlyph(glyph);
            if(!glyph->width) // Empty glyph
            {
                delete glyph;
                return ReplyOk;
            }
            // now we have our full glyph, try harder to split it
_nextglyph:
            raw=new int[glyph->height];            
            if(adm_estimate_glyphSize(glyph,&minx, &maxx,&miny,&maxy,raw))
            {
            //printf("Glyph width :%lu min:%lu max:%lu estimate width:%lu\n",glyph->width,minx,maxx,maxx-minx+1);
            if((maxx-minx+2)<glyph->width && (maxx-minx>2) && (maxy-miny>2))
            {
                // Suspicously too small
                // We have to split the glyph
                // recursively to extract each glyph
                uint32_t width=maxx-minx+1;
                uint32_t defStride=width+1;
                
                if(defStride>glyph->width) defStride=glyph->width;
                
                admGlyph *lefty=new admGlyph(defStride,glyph->height);
                for(int32_t i=miny;i<=maxy;i++)
                {
                    if(raw[i]!=-1) memcpy(&(lefty->data[0+i*defStride]),&(glyph->data[minx+i*glyph->width]),raw[i]+1-minx);
                    else
                            memcpy(&(lefty->data[0+i*defStride]),&(glyph->data[minx+i*glyph->width]),defStride);
                }
                lefty=clippedGlyph(lefty);
              
                {
                    // Remove that from the original
                    for(uint32_t i=0;i<glyph->height;i++)
                    {
                        //printf("%d:%d(%d)\n",i,raw[i],glyph->width);
                        if(raw[i]!=-1) memset(&(glyph->data[i*glyph->width]),0,raw[i]+1);
                        else           memset(&(glyph->data[i*glyph->width]),0,defStride); 
                    }
                    // Clip
                    glyph=clippedGlyph(glyph);
                
                    if(lefty->width)
                    {
                        reply=glyphToText(lefty,head,decodedString);
                        if(reply!=ReplyOk)
                        {
                            printf("Glyph2text failed(1)\n");
                            return reply;
                        }
                    }
                    else
                        delete lefty;
                    if(glyph->width)
                    {
                        if(raw) delete [] raw;
                        goto _nextglyph;                    
                    } 
                 }           
            }
            }//If
            if(raw) delete [] raw;
            if(glyph->width)
            {
                reply=glyphToText(glyph,head,decodedString);
                if(reply!=ReplyOk)                 
                {
                    printf("Glyph2text failed(2)\n");
                    return reply;
                }
            }
            else 
            {
                delete glyph;
            }
            
    return ReplyOk;

}
/**
      \fn ocrUpdateMinThreshold
      \brief update the threshold to say black or white when we merge bitmap and alpha mask
*/
void ocrUpdateMinThreshold(void)
{
        int val;
        val=minThreshold;
        if(DIA_GetIntegerValue(&val, 0x30, 0x80, "Minimum pixel value", "Enter new minimum pixel"))
        {
                minThreshold=val;

        }
}
//EOF
