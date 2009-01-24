/***************************************************************************
 *   Copyright (C) 2007 by fx   *
 *   fx@debian64   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#define ADM_assert assert
#include "ADM_ebml.h"
#include "mkv_tags.h"

void ADM_mkvWalk(ADM_ebml_file *working, uint32_t size);

int main(int argc, char *argv[])
{
  ADM_ebml_file *ebml=new ADM_ebml_file ;
  uint64_t id,len;
  ADM_MKV_TYPE type;
  const char *ss;
  
  if(!ebml->open(argv[1])) ADM_assert(0);
  
  // Read level 1 stuff
  while(!ebml->finished())
  {
      ebml->readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[MKV] Tag 0x%x not found\n",id);
        ebml->skip(len);
        continue;
      }
      uint64_t w=ebml->tell();
      printf("Found Tag : %x (%s) at 0x%x size %d end at 0x%x\n",id,ss,w,len,w+len);
      if(type==ADM_MKV_TYPE_CONTAINER)
      {
        ADM_mkvWalk(ebml,len);
        if(ebml->tell() > w+len)
        {
                        printf("*** WARNING INCORRECT CONTAINER SIZE : %d vs real size %d\n",len,ebml->tell()-w);
        }
        else
        {
                printf(">Seeking from 0x%x to  0x%x (size %d)\n",w,w+len,len);
                ebml->seek(w+len);
        }
      }else
        ebml->skip(len);
  }
  return 0;
}
/**

*/
#define recTab() for(int pretty=0;pretty<recurse;pretty++) printf("\t");
void ADM_mkvWalk(ADM_ebml_file *working, uint32_t size)
{
  uint64_t id,len;
  ADM_MKV_TYPE type;
  const char *ss;
  static int recurse=0;
  uint64_t pos; 
  
  recurse++;
   ADM_ebml_file son(working,size);
   while(!son.finished())
   {
      pos=son.tell();
      son.readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        recTab();printf("[MKV] Tag 0x%x not found\n",id);
        son.skip(len);
        continue;
      }
      recTab();printf("at 0x%llx, Found Tag : %x (%s) type %d (%s) size %d, start at 0x%x end at 0x%x\n",pos,id,ss,type,ADM_mkvTypeAsString(type),len,working->tell(),working->tell()+len);
      uint32_t val;
      switch(type)
      {
        case ADM_MKV_TYPE_CONTAINER:
                  //if(id!=MKV_CLUSTER)
                  if(len)
                    ADM_mkvWalk(&son,len);
                    else
                    {
                                printf("******************************* WARNING ZERO SIZE ******************\n");
                    }

                  //else
                  //    son.skip(len);
                  break;
        case ADM_MKV_TYPE_UINTEGER:
                  val=son.readUnsignedInt(len);
                  recTab();printf("\tval uint: %llu (0x%llx) \n",val,val);
                  break;
        case ADM_MKV_TYPE_UTF8:
        {
                  char string[len+1];
                  string[0]=0;
                  son.readString(string,len);
                  recTab();printf("\tval utf8 as string: <%s> \n",string);
        }
                  break;
                  
        case ADM_MKV_TYPE_FLOAT:
                  recTab();printf("\tval float: %f \n",son.readFloat(len));
                  break;
        case ADM_MKV_TYPE_INTEGER:
                  recTab();printf("\tval int: %lld \n",son.readSignedInt(len));
                  break;
        case ADM_MKV_TYPE_STRING:
        {
                  char string[len+1];
                  string[0]=0;
                  son.readString(string,len);
                  recTab();printf("\tval string: <%s> \n",string);
                  break;
        }
        default:
                if(id==MKV_BLOCK)
                {
                        recTab();printf("\t\tTrack :%u",son.readu8()-128); // Assume 1 byte code
                        //printf(" Timecode:%d",son.reads16());
                        son.skip(2);
                        int lacing=son.readu8();
                        printf(" Lacing :%x ");
                        if(lacing&1) printf(" keyframe ");
                        lacing=(lacing>>1)&3;
                        switch(lacing)
                        {
                                       case 0:printf("No lacing\n");break;
                                       case 1:printf("Xiph lacing\n");break;
                                       case 3:printf("Ebml lacing\n");break;
                                       case 2:printf("Fixed lacing :%u remaining:%u\n",son.readu8()+1,len-5);len--;break;

                        }
                        son.skip(len-4);

                }
                else
                {
                        recTab();printf("Skipping %s\n",ss);
                        son.skip(len);
                }
                break;
      }
   }
   recurse--;
}

void bigHexPrint(uint64_t v)
{
  int s=56;
  int n=0,z;
  
  for(int i=0;i<8;i++)
  {
    z=(v>>s)&0xff;
    if(!z && !n) 
    {
      
    }else
    {
      printf("[%02x]", (v>>s)&0xff);
      n=1;
    }
    s-=8;
  }
  printf("\n"); 
}
extern "C"
{
double av_int2dbl(int64_t v)
{
    if(v+v > 0xFFEULL<<52)
            return 0;
    return ldexp(((v&((1LL<<52)-1)) + (1LL<<52)) * (v>>63|1), (v>>52&0x7FF)-1075);
}

float av_int2flt(int32_t v)
{
    if(v+v > 0xFF000000U)
            return 0;
return ldexp(((v&0x7FFFFF) + (1<<23)) * (v>>31|1), (v>>23&0xFF)-150);
}
}
//EOF
