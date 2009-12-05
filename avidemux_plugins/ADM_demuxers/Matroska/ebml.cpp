/***************************************************************************
 *   Copyright (C) 2007 by mean,    *
 *   fixounet@free.fr   *
 *                                                                         *
 *        EBML Handling code                                               *
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

#include "ADM_default.h"
#include "ADM_ebml.h"
#define aprintf(...) {}

#include "ADM_assert.h"
#if 0
#define vprintf printf
#else
#define vprintf(...) {}
#endif

extern "C"
{
  double av_int2dbl(int64_t v);
  float av_int2flt(int32_t v);
}

/*
  It is slow , optimize later
*/

uint8_t    ADM_ebml::readElemId(uint64_t *code,uint64_t *len)
{
  *code=readEBMCode_Full(); // Keep the length-bits, easier to match with documentation
  *len=readEBMCode();
  return 1;
}

/**
      \fn       readEBMCode
      \brief    Returns unsigned integer
*/

uint64_t    ADM_ebml::readEBMCode(void)
{
 uint64_t start=readu8();
  uint64_t val;
  uint32_t mask=0x80,outmask=0x7F,more=0;

  aprintf("Start :%x at %llx\n",start,tell()-1);

  while(!(mask&start))
  {
    mask>>=1;
    ADM_assert(mask);
    more++;
  }
  outmask=mask-1;
  start=start&outmask;
  for(int i=0;i<more;i++)
  {
    start=(start<<8)+readu8();
  }
  aprintf("End at %llx\n",tell());
  return start;
}
/**
      \fn       readEBMCode
      \brief    Returns unsigned integer
*/

int64_t    ADM_ebml::readEBMCode_Signed(void)
{
  uint8_t start=readu8();
  int64_t val;
  uint32_t mask=0x80,outmask=0x7F,more=0;

  while(!(mask&start))
  {
    mask>>=1;
    ADM_assert(mask);
    more++;
  }
  outmask=mask-1;
  val=start&outmask;
  for(int i=0;i<more;i++)
  {
    val=(val<<8)+readu8();
  }
  // Signed !

  switch(more)
  {
    case 0: val-=63;break;
    case 1: val-=8191;break;
    case 2: val-=1048575L;break;
    default:
        ADM_assert(0);
        return 0;

  }

  return val;
}
/**
      \fn       readEBMCode_Full
      \brief    Returns complete code, including size-bits (used only for EBML_ID)
*/
uint64_t    ADM_ebml::readEBMCode_Full(void)
{
  uint64_t start=readu8();
  uint32_t mask=0x80,more=0;
  aprintf(">>StartFull :%x at %llx\n",start,tell()-1);
  while(!(mask&start))
  {
    mask>>=1;
    ADM_assert(mask);
    more++;
  }
  for(int i=0;i<more;i++)
  {
    start=(start<<8)+readu8();
  }
  return start;
}

uint64_t    ADM_ebml::readUnsignedInt(uint32_t nb)
{
  uint64_t val=0;
  for(int i=0;i<nb;i++)
  {
    val=(val<<8)+readu8();
  }
  return val;
}
int64_t    ADM_ebml::readSignedInt(uint32_t nb)
{
  int64_t val=0;
  uint8_t r=0;
  r=readu8();
  if(r&0x80) // sign
      val=-1;
  val=(val<<8)+r;
  for(int i=0;i<nb-1;i++)
  {
    r=readu8();
    val=(val<<8)+r;
  }
  return val;

}
uint8_t     ADM_ebml::readString(char *string, uint32_t maxLen)
{
  uint8_t v;
  while(maxLen--)
  {
    v=*string++=readu8();
    if(!v) return 1;
  }
  *string=0;
  return 1;
}
/******************
  Low level read
**********************/


uint8_t ADM_ebml::readu8(void)
{
  uint8_t v;
    readBin(&v,1);
    return v;
}
uint16_t ADM_ebml::readu16(void)
{
  uint8_t v[2];
    readBin(v,2);
    return (uint16_t)(v[0]<<8)+v[1];
}
/**
*/
float       ADM_ebml::readFloat(uint32_t n)
{
  if(n!=4 && n!=8) ADM_assert(0);

  switch(n)
  {
    case 4:
    {
        uint32_t u4=readUnsignedInt(4);
        return av_int2flt(u4);
      }
    case 8:
    {
        uint64_t u8=readUnsignedInt(8);
        return  av_int2dbl(u8);
    }
    default:
        ADM_assert(0);
  }
}

ADM_ebml::ADM_ebml(void)
{

}
ADM_ebml::~ADM_ebml()
{

}


//*******************************************
//***********FILE IO PART *******************
//*******************************************
ADM_ebml_file::ADM_ebml_file(ADM_ebml_file *father,uint64_t size)
{
  _close=0;
  _size=size;
  fp=father->fp;
  _fileSize=father->_fileSize;
   _begin=ftello(fp);
   _root=father->_root;
   ADM_assert(_root);
   _root->_refCount++;
}
ADM_ebml_file::ADM_ebml_file(void) : ADM_ebml()
{
  _close=0;
  fp=NULL;
  _fileSize=0;
  _begin=0;
  _root=NULL;
  _refCount=0;
}
ADM_ebml_file::~ADM_ebml_file()
{
  ADM_assert(fp);
  if(_close)  // We are the 1st one
  {
    ADM_assert(!_begin);
    if(!_refCount)
    {
      fclose(fp);
    }else
    {
      printf("WARNING: EBML killing father with non empty refcount : %u\n",_refCount);
    }
  }
  else
  {
    fseeko(fp,_begin+_size,SEEK_SET);
    ADM_assert(_root);
    _root->_refCount--;
  }
  fp=NULL;
}
uint8_t ADM_ebml_file::open(const char *name)
{

  fp=ADM_fopen(name,"rb");
  if(!fp)
  {
    aprintf("[EBML FILE] Failed to open <%s>\n",name);
    return 0;
  }
  _root=this;
  _close=1;
  fseeko(fp,0,SEEK_END);
  _begin=0;
  _fileSize=_size=ftello(fp);
  fseeko(fp,0,SEEK_SET);
  return 1;
}
uint8_t  ADM_ebml_file::readBin(uint8_t *whereto,uint32_t len)
{
  ADM_assert(fp);
  if(!fread(whereto,len,1,fp)) return 0;
  return 1;
}

uint8_t ADM_ebml_file::skip(uint32_t vv)
{
  fseeko(fp,vv,SEEK_CUR);
  return 1;
}
uint64_t ADM_ebml_file::tell(void)
{
  return ftello(fp);
}
uint8_t ADM_ebml_file::seek(uint64_t pos)
{
  fseeko(fp,pos,SEEK_SET);
  return 1;
}
uint8_t ADM_ebml_file::finished(void)
{
  if(tell()>(_fileSize-2)) return 1;
  if(tell()>(_begin+_size-2)) return 1;
  return 0;
}
/**
  \fn find
  \brief Search for the tag given and returns the corresponding atom
*/
 uint8_t ADM_ebml_file::find(ADM_MKV_SEARCHTYPE search,MKV_ELEM_ID  prim,MKV_ELEM_ID second,uint64_t *len,uint32_t rewind)
{
  uint64_t id,pos;
  ADM_MKV_TYPE type;
  const char *ss;

    vprintf("[MKV]Searching for tag %llx %llx\n",prim,second);
    if(rewind) seek(_begin);
    if(search==ADM_MKV_PRIMARY)
    {
          return simplefind(prim,len,rewind);
      }
    vprintf("[MKV]Searching primary : %llx\n",prim);
    if(!simplefind(prim,len,rewind))
    {
      vprintf("[MKV] Primary find failed for %llx\n",prim);
      return 0;
    }
    // Now we have the father, go inside
    ADM_ebml_file *son=new ADM_ebml_file(this,*len);
    vprintf("[MKV]Searching secondary : %llx\n",second);
    if(!son->simplefind(second,len))
    {
      vprintf("[MKV] secondary find failed for secondary %llx\n",second);
      delete son;
      return 0;
    }
    pos=son->tell();
    delete son;
    seek(pos);
    return 1;
}

/**
  \fn find
  \brief Search for the tag given and returns the corresponding atom
*/
uint8_t ADM_ebml_file::simplefind(MKV_ELEM_ID  prim,uint64_t *len,uint32_t rewind)
{
  uint64_t id,alen;
  ADM_MKV_TYPE type;
  const char *ss;


    vprintf("[MKV] Simple Searching for tag %llx\n",prim);
    if(rewind) seek(_begin);

      while(!finished())
      {
          readElemId(&id,&alen);
          if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
          {
              vprintf("[MKV] Tag 0x%x not found\n",id);
              skip(alen);
              continue;
           }
          if(!alen)
          {
            printf("[MKV] WARNING ZERO SIZED ATOM %s %"LLU"/%"LLU"\n",ss,tell(),_fileSize);
            continue;
          }
          vprintf("Found Tag : %x (%s)\n",id,ss);
          if(id==prim)
          {
            *len=alen;
            return 1;
          }else
            skip(alen);
      }
    vprintf("[MKV] Failed to locate %llx\n",prim);
    return 0;
}
/**
    \fn remaining
    \brief returns the # of bytes remaining in this atom
*/
uint64_t ADM_ebml_file::remaining(void)
{
  uint64_t pos=tell();
  ADM_assert(pos<=(_begin+_size));
  return (_begin+_size)-pos;
}
//EOF
