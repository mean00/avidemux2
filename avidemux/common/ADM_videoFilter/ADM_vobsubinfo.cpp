//
// C++ Implementation: vobsub parser
//
// Description:
//
//
// Author: Mean, 2005 GPL
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
#include "ADM_vobsubinfo.h"

static uint8_t fillLine(char *str,VobSubInfo *sub,uint32_t line);
static uint32_t countLine(FILE *f,int index);
uint8_t fillPalette(char *str,VobSubInfo *sub);

uint8_t destroySubInfo(VobSubInfo *sub)
{
        if(!sub) return 1;      // ?
        if(sub->lines) delete [] sub->lines;
        delete sub;
        return 1;


}
//****************** 
// Extract Line info
//******************
uint8_t fillLine(char *str,VobSubInfo *sub,uint32_t line)
{
int hh,mm,ss,ms,o;
uint64_t ti;
uint32_t pos;

        ADM_assert(line<sub->nbLines);

        o=sscanf(str,"timestamp: %d:%d:%d:%d, filepos: %x\n",&hh,&mm,&ss,&ms,&pos);
        ADM_assert(o==5);
        
        ti=hh*3600+mm*60+ss;
        ti=ti*1000+ms;
        
        sub->lines[line].startTime      =ti;
        sub->lines[line].stopTime       =ti+1000;
        sub->lines[line].fileOffset     =pos;
        
        return 1;
}
//******************
// Extract palette
//******************
uint8_t fillPalette(char *str,VobSubInfo *sub)
{
int p[16],o;

        o=sscanf(str,"palette: %x, %x, %x, %x, %x, %x, %x, %x,"
                                " %x, %x, %x, %x, %x, %x, %x, %x",
                                &p[0],&p[1],&p[2],&p[3],&p[4],&p[5],&p[6],&p[7],
                                &p[8],&p[9],&p[10],&p[11],&p[12],&p[13],&p[14],&p[15]);
        ADM_assert(o==16);
        for(int i=0;i<16;i++)
                sub->Palette[i]=p[i];
                                       
        return 1;
}
//******************
//******************
uint8_t vobSubRead(char *filename,int index,VobSubInfo **info)
{
FILE            *file=NULL;
uint32_t        nb_lines;
VobSubInfo      *sub=NULL;
uint8_t         success=0;
uint32_t        line=0,l;
char            str[1024];
char            *dup;
int             language=0;

        if(!filename)
        {
                printf("Null file ?\n");
                return 0;
        }
        *info=NULL;
        file=fopen(filename,"rt");
        if(!file) 
        {
                printf("Could not open %s file\n",filename);
                return 0;
        }
        nb_lines=countLine(file,index);
        if(!nb_lines)
        {
                printf("Empty file\n");
                 goto subAbort;
        }
        // Try to read the file
        sub=new VobSubInfo;
        memset(sub,0,sizeof(VobSubInfo));
        //
        sub->nbLines=nb_lines;
        sub->lines=new vobSubLine[nb_lines];
        memset(sub->lines,0,sizeof(vobSubLine)*nb_lines);
        printf("Rebuilding %d lines of subs\n",nb_lines);
        
        while(line<nb_lines && !feof(file))
        {
                fgets(str,1023,file); 
                if(!strncmp(str,"palette:",7))
                {
                                 fillPalette(str,sub);
                                 sub->hasPalette=1;
                }
                else 
                {
                        if(!strncmp(str,"timestamp: ",10) && language)        
                        {
                                fillLine(str,sub,line);
                                line++;
                        }
                        else
                        {
                                if(!strncmp(str,"id:",3))       // Catch language/index
                                {
                                  int  l;
                                  char s[50];
                                  s[0]=0;
                                  l=999;
                                  sscanf(str,"id: %s index: %d",s,&l);
                                  printf("Found lang : %s index %d while searching %d\n",s,l,index);
                                  if(l==index)
                                  {
                                	  language=1;
                                	  printf("Match\n");
                                  }
                                  else language=0;                                                                              
                                
                                }
                                else
                                {
                                        if(!strncmp(str,"size:",5))       // Catch original screen dimension
                                        {
                                            sscanf(str,"size:%"SCNu32"x%"SCNu32"",&(sub->width),&(sub->height));
                                        }
                                
                                }
                                
                        }
                }
        }
subSuccess:        
        success=1;
        if(!sub->hasPalette)
        {
            for(int j=0;j<16;j++)
                sub->Palette[j]=j;   
        }
subAbort:        
        if(success)
        {
                *info=sub;
        }
        else
        {
                destroySubInfo( sub);        
        }
        fclose(file);
        return success;

}
//******************
// count #nb beginning by timestamp in file
//******************
uint32_t countLine(FILE *f,int index)
{
char str[1024],s[1024];
uint32_t nb=0;
uint32_t match=0;
int lang;

        fseek(f,0,SEEK_SET);
        while(!feof(f))
        {
                fgets(str,1023,f);
                if(!strncmp(str,"id:",3))
                {
                  s[0]=0;
                  lang=9999;
                  sscanf(str,"id: %s index: %d",s,&lang);
                  if(lang==index) match=1;
                  else match=0;
                }
                else
                  if(match)
                        if(!strncmp(str,"timestamp: ",10)) nb++;
        
        }
        fseek(f,0,SEEK_SET);
        return nb;
}
//*****************************************
vobSubLanguage *vobSubAllocateLanguage(void)
{
  vobSubLanguage *l;
  l=new vobSubLanguage;
  memset(l,0,sizeof(vobSubLanguage));
  return l;  
}
//*****************************************
uint8_t vobSubDestroyLanguage(vobSubLanguage *lingua)
{
  for(uint32_t i=0;i<lingua->nbLanguage;i++)  
    delete [] lingua->language[i].name;
  delete lingua; 
  return 1;
}
//*****************************************
uint8_t vobSubGetLanguage(const char *filename,vobSubLanguage *lingua)
{
  char str[1024];
  char s[16];
  uint32_t nb=0,index;
  FILE *fd=NULL;
  
  fd=fopen(filename,"rb");
  if(!fd) return 0;
  
  

  
  while(!feof(fd))
  {
    fgets(str,1023,fd);
    if(!strncmp(str,"id: ",3))
    {
      sscanf(str,"id: %s index: %d",s,&index);
      lingua->language[nb].name=new char[3];
      lingua->language[nb].name[0]=str[4];
      lingua->language[nb].name[1]=str[5];
      lingua->language[nb].name[2]=0;
      //
      lingua->language[nb].index=index;
      nb++;
    }        
  }
  lingua->nbLanguage=nb;
  return 1;
}

//EOF
