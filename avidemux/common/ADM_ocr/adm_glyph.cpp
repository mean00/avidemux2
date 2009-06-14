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
 #include "adm_glyph.h"
#include "DIA_coreToolkit.h"
 //*************************************************
admGlyph::admGlyph(uint32_t w,uint32_t h)
{
    ADM_assert(w*h);
    width=w;
    height=h;
    data=new uint8_t[w*h];
    memset(data,0,w*h);
    code=NULL;
    next=NULL;
}
//*************************************************
admGlyph::~admGlyph()
{
    delete [] data;
    data=NULL;
    if(code)
    {
        ADM_dealloc(code);
        code=NULL;
    }
}
//*************************************************
uint8_t admGlyph::create(uint8_t *incoming, uint32_t stride)
{
uint8_t *in=incoming;
uint8_t *out=data;
    for(uint32_t y=0;y<height;y++)
    {
        memcpy(out,in,width);
        out+=width;
        in+=stride;
    }
    return 1;
}
//*************************************************
uint8_t  insertInGlyphTree(admGlyph *startGlyph, admGlyph *candidate)
{
admGlyph *old;
            old=startGlyph->next;
            candidate->next=old;
            startGlyph->next=candidate;
            return 1;
}
//*************************************************
uint8_t  destroyGlyphTree(admGlyph *startGlyph)
{
admGlyph *head,*tmp;
        if(!startGlyph) return 0;
        head=startGlyph->next;
        while(head)
        {
            tmp=head;
            head=head->next;
            delete tmp;
        }
    return 1;
}
//*************************************************
admGlyph *searchGlyph(admGlyph *startGlyph, admGlyph *candidate)
{
admGlyph *head=startGlyph->next;
            while(head)
            {   
                if(head->width==candidate->width && head->height==candidate->height)
                {
                    //Raw compare
                    if(!memcmp(head->data,candidate->data,head->width*head->height))
                        return head;
                
                }
                head=head->next;
            }
            return NULL;
}
/*************************************************/
admGlyph *clippedGlyph(admGlyph *in)
{
uint32_t w,h,lonecount,lone;
int32_t top,bottom,left,right;
admGlyph *nw=NULL;
            w=in->width;
            h=in->height;
            // Look if we got a lonely point at the first line
            lonecount=0;
            if(w>3)
              for(uint32_t i=0;i<w;i++) 
                if(in->data[i]) 
                {
                    lonecount++;
                    lone=i;
                }
            if(lonecount==1)
            {
                if(!lone) lone++;
                if(lone==w-1) lone--;
                if(!in->data[w+lone-1] && !in->data[w+lone] && !in->data[w+lone+1])
                    in->data[lone]=0;
            }
            // Go!
            left=0;
            while(columnEmpty(in->data+left,w,h) && left<w) left++;
            if(left==w) 
            {
                in->width=in->height=0;
                return in;
            }
            right=w-1;
            while(columnEmpty(in->data+right,w,h) && right>=left) right--;
            
            top=0;
            while(lineEmpty(in->data,w,w,top) && top<h) top++;
            
            bottom=h-1;
            while(lineEmpty(in->data,w,w,bottom) && bottom>=top) bottom--;
            
            nw=new admGlyph(right-left+1,bottom-top+1);
            nw->create(in->data+left+top*w,w);
            delete in;
            return nw;

}
/**
    \fn glyphSearchFather
    \brief Returns the father of the "in" glyph.
    @param in : Glyph to search the father of
    @param head : head of glyph list
    @returns father or NULL if not found
*/
admGlyph *glyphSearchFather(admGlyph *in,admGlyph *head )
{
  admGlyph *cur=head;
  while(cur)
  {
    if(!cur->next) return NULL;
    if(cur->next==in) return cur; 
    cur=cur->next;
  }
  return NULL;
}
/*************************************************/
/**
    \fn       glyphSave
    \brief    Save the glypset
*/
uint8_t saveGlyph(char *name,admGlyph *head,uint32_t nb)
{
  FILE *out;
  uint32_t slen;
  nb=0;
  admGlyph *glyph=head->next;
    
    
  out=fopen(name,"wb");
  if(!out)
  {
    GUI_Error_HIG(QT_TR_NOOP("Could not write the file"), NULL);
    return 0;
  }
    
    /* First count how many glyphs */
     while(glyph)
    {
      glyph=glyph->next;
      nb++;
    }
    
#define WRITE(x) fwrite(&(x),sizeof(x),1,out);
    WRITE(nb);
    
    glyph=head->next;
    while(glyph)
    {
      WRITE(glyph->width);
      WRITE(glyph->height);
      fwrite(glyph->data,glyph->width*glyph->height,1,out);
      if(glyph->code) slen=strlen(glyph->code);
      else slen=0;
      WRITE(slen);
      fwrite(glyph->code,slen,1,out);
      glyph=glyph->next;
    }
    printf("[Glyph] Saved %u glyphs\n",nb);
    fclose(out);
    return 1;
  
}
/**
    \fn loadGlyph
    \brief Load a glyph set
*/
uint8_t loadGlyph(char *name,admGlyph *head,uint32_t *outNb)
{
  FILE *out;
  admGlyph *glyph,*nw;
  uint32_t N,w,h,slen;
  uint32_t nbGlyphs;
 
  *outNb=0;
  
  glyph=head;
  out=fopen(name,"rb");
  if(!out)
  {
    GUI_Error_HIG(QT_TR_NOOP("File error"), QT_TR_NOOP("Could not read \"%s\"."), name);
    return 0;
  }
#define READ(x) fread(&(x),sizeof(x),1,out);
    nbGlyphs=0;
    READ(N);
    while(N--)
    {
        
      READ(w);
      READ(h);
      nw=new admGlyph(w,h);
      fread(nw->data,w*h,1,out);
      READ(slen);
      if(slen)
      {
        nw->code=new char[slen+1];
        fread(nw->code,slen,1,out);
        nw->code[slen]=0;
      }
      glyph->next=nw;
      glyph=nw;
      nbGlyphs++;
    }
    
    fclose(out);
    *outNb=nbGlyphs;
    printf("[Glyph] Loaded %u glyphs\n",nbGlyphs);
    return 1;

}
/*************************************************/

