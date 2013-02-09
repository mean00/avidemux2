/***************************************************************************
                          ADM_infoextractor
                             -------------------
           - extract additionnal info from header (mp4/h263)                  
**************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_Video.h"
#include "fourcc.h"
#include "ADM_getbits.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"
#include "ADM_coreUtils.h"

extern "C"
{
#include "libavutil/common.h"
}
#define aprintf(...) {}

#define MP4_VOL         0x20
#define MP4_VO_SEQ      0xB0
#define MP4_USER_DATA   0xB2
#define MP4_GOP         0xB3
#define MP4_VISUAL_OBJ  0xB5
#define MP4_VOP         0xB6         

/*
    Extract width & height from vol header passed as arg
*/
uint8_t
extractMpeg4Info (uint8_t * data, uint32_t dataSize, uint32_t * w,
		  uint32_t * h, uint32_t * time_inc)
{
  // Search startcode
  uint8_t b;
  uint32_t idx = 0;
  uint32_t mw, mh;
  uint32_t timeVal;

  //mixDump(data,dataSize);
  //printf("\n");
  while (1)
    {
      uint32_t startcode = 0xffffffff;
      while (dataSize > 2)
	{
	  startcode = (startcode << 8) + data[idx];
	  idx++;
	  dataSize--;
	  if ((startcode & 0xffffff) == 1)
	    break;
	}
      if (dataSize > 2)
	{
	  //printf("Startcodec:%x\n",data[idx]);
	  if ((data[idx] & 0xF0) == MP4_VOL)	//VOL start
	    {
	      dataSize--;
	      idx++;
#if 0
	      printf ("VOL Header:\n");

	      if (dataSize < 16)
		{
		  mixDump (data + idx, dataSize);
		  printf ("\n");
		}
	      else
		{
		  mixDump (data + idx, 16);
		  printf ("\n");
		}
#endif
	      // Here we go !
          getBits bits(dataSize,data + idx);

	      //
	      bits.skip(1);
          bits.skip(8);	      // Obj type indication
	      if (bits.get(1))	// VO od 
		{
		  bits.get( 4);	// Ver
		  bits.get( 3);	// Priority
		}
	      if (bits.get( 4) == 15)	// custom A/R
		{
		  bits.get( 8);
		  bits.get( 8);
		}
	      if (bits.get( 1))	// Vol control param
		{
		  bits.get( 2);	//Chroma
		  bits.get( 1);	// Low delay
		  if (bits.get( 1))	// VBV Info
		    {
		      bits.get( 16);
		      bits.get( 16);
		      bits.get( 16);
		      bits.get( 15);
		      bits.get( 16);
		    }
		}
	      bits.get( 2);	//  Shape
	      bits.get( 1);	//  Marker
	      timeVal = bits.get( 16);	// Time increment
	      *time_inc = log2 (timeVal - 1) + 1;
	      if (*time_inc < 1)
		 *time_inc = 1;
	      bits.get( 1);	//  Marker
	      if (bits.get( 1))	// Fixed vop rate, compute how much bits needed
		{
		  bits.get( *time_inc);
		}
	      bits.get( 1);	//  Marker
	      mw = bits.get( 13);
	      bits.get( 1);	//  Marker
	      mh = bits.get( 13);
	      // /Here we go
	      //printf("%d x %d \n",mw,mh);
	      *h = mh;
	      *w = mw;
	      return 1;;
	      // Free get bits ?
	      // WTF ?
	    }
	  continue;
	}
      else
	{
	  printf ("No more startcode\n");
	  // Free get bits ?
	  return 0;

	}
    }

  return 0;
}
/**
    \fn extractVopInfo
    \brief extract info from vop : Vop type, module time base, time inc
    
    Warning this function expects data to start AFTER startcode, contrarily to other functions here!
*/

uint8_t
extractVopInfo (uint8_t * data, uint32_t len, uint32_t timeincbits,
		uint32_t * vopType, uint32_t * modulo, uint32_t * time_inc,
		uint32_t * vopcoded)
{
  
  int vop;
  uint32_t vp, tinc;
  getBits bits(len,data );
  
  vop = bits.get( 2);
  switch (vop)
    {
    case 0:
      vp = AVI_KEY_FRAME;
      break;
    case 1:
      vp = 0;
      break;
    case 2:
      vp = AVI_B_FRAME;
      break;
    case 3:
      vp = 0;
      break;			// D FRAME ????
    default:
      printf ("Unknown vop type :%d\n", vop);
      return 0;
    }
  /* Read modulo */
  int imodulo = 0;
  while (bits.get(1) != 0)
    imodulo++;
  if (!bits.get(1))
    {
      printf ("Wrong marker1\n");
      return 0;
    }

  /* Read time */
  tinc = bits.get(timeincbits);
  /* Marker */
  if (!bits.get(1))
    {
      printf ("Wrong marker2\n");
      return 0;
    }
  /* Vop coded */
  *modulo = imodulo;
  *vopcoded = bits.get(1);
  *vopType = vp;
  *time_inc = tinc;
  return 1;
}
/**
      \brief extractH263FLVInfo
      \fn Extract width/height from FLV header
*/
uint8_t
extractH263FLVInfo (uint8_t * buffer, uint32_t len, uint32_t * w,
		    uint32_t * h)
{
  
  int format;
  getBits bits(len,buffer );
  
  if (bits.get( 17) != 1)
    {
      printf ("[FLV]Wrong FLV1 header\n");
      return 0;
    }
  format = bits.get( 5);
  if (format != 0 && format != 1)
    {
      printf ("[FLV]Wrong FLV1 header format\n");
      return 0;
    }

  bits.get(8);		/* picture timestamp */
  format = bits.get( 3);
  switch (format)
    {
    case 0:
      *w = bits.get( 8);
      *h = bits.get( 8);
      break;
    case 1:
      *w = bits.get( 16);
      *h = bits.get( 16);
      break;
    case 2:
      *w = 352;
      *h = 288;
      break;
    case 3:
      *w = 176;
      *h = 144;
      break;
    case 4:
      *w = 128;
      *h = 96;
      break;
    case 5:
      *w = 320;
      *h = 240;
      break;
    case 6:
      *w = 160;
      *h = 120;
      break;
    default:
      printf ("[FLV]Wrong width format\n");
      return 0;
      break;
    }
  return 1;
}
/**
    \fn extractH263Info
    \brief  Extract H263 width & height from header

*/
uint8_t
extractH263Info (uint8_t * data, uint32_t dataSize, uint32_t * w,
		 uint32_t * h)
{
  uint32_t val;
  getBits bits(dataSize,data );
  

  mixDump (data, 10);
  val = bits.get( 16);
  if (val)
    {
      printf ("incorrect H263 header sync\n");
      return 0;
    }
  val = bits.get( 6);
  if (val != 0x20)
    {
      printf ("incorrect H263 header sync (2)\n");
      return 0;
    }
  //
  bits.get( 8);		// timestamps in 30 fps tick
  bits.get( 1);		// Marker
  bits.get( 1);		// Id
  bits.get( 1);		// Split
  bits.get( 1);		// Document Camera indicator
  bits.get( 1);		// Full Picture Freeze Release
  val = bits.get( 3);
  switch (val)
    {

    case 1:
      *w = 128;
      *h = 96;
      return 1;
      break;
    case 2:
      *w = 176;
      *h = 144;
      return 1;
      break;
    case 6:
    case 7:
      printf ("H263+:Todo\n");
    default:
      printf ("Invalid format\n");
      return 0;
      break;
    }
  return 0;
}
/**
    \fn ADM_searchVop
    \brief search vop header in a bitstream 
    Used for packed bitstream and also to identify the frametype

*/
uint32_t
ADM_searchVop (uint8_t * begin, uint8_t * end, uint32_t * nb, ADM_vopS * vop,
	       uint32_t * timeincbits)
{

  uint32_t off = 0;
  uint32_t globalOff = 0;
  uint32_t voptype;
  uint8_t code;
  uint32_t w, h, t;
  uint32_t modulo, time_inc, vopcoded, vopType;
  *nb = 0;
  while (begin < end - 3)
    {
      if (ADM_findMpegStartCode (begin, end, &code, &off))
	{
	  if (code == MP4_VOP)
	    {
	      // Analyse a bit the vop header
	      uint8_t coding_type = begin[off];
	      coding_type >>= 6;
	      aprintf ("\t at %u %d Img type:%s\n", off, *nb,
		       s_voptype[coding_type]);
	      switch (coding_type)
		{
		case 0:
		  voptype = AVI_KEY_FRAME;
		  break;
		case 1:
		  voptype = 0;
		  break;
		case 2:
		  voptype = AVI_B_FRAME;
		  break;
		case 3:
		  printf ("[Avi] Glouglou\n");
		  voptype = 0;
		  break;

		}
	      vop[*nb].offset = globalOff + off - 4;
	      vop[*nb].type = voptype;



	      /* Get more info */
	      if (extractVopInfo
		  (begin + off, end - begin - off, *timeincbits, &vopType,
		   &modulo, &time_inc, &vopcoded))
		{
		  aprintf
		    (" frame found: vopType:%x modulo:%d time_inc:%d vopcoded:%d\n",
		     vopType, modulo, time_inc, vopcoded);
		  vop[*nb].modulo = modulo;
		  vop[*nb].timeInc = time_inc;
		  vop[*nb].vopCoded = vopcoded;
		}
	      *nb = (*nb) + 1;
	      begin += off + 1;
	      globalOff += off + 1;
	      continue;

	    }
	  else if (code == 0x20 && off >= 4)	// Vol start
	    {

	      if (extractMpeg4Info
		  (begin + off - 4, end + 4 - off - begin, &w, &h,
		   timeincbits))
		{
		  aprintf ("Found Vol header : w:%d h:%d timeincbits:%d\n", w,
			   h, *timeincbits);
		}

	    }
	  begin += off;
	  globalOff += off;
	  continue;
	}
      return 1;
    }
  return 1;
}

typedef struct
{
        uint32_t code;
        uint8_t  *data;
        uint32_t len;
}mpeg4unit;
#define MKVOL(x) {x,#x}
typedef struct
{
    int unitId;
    const char *unitName;
}unitDesc;
static const unitDesc descriptor[]={
MKVOL(MP4_VOL         ),
MKVOL(MP4_VO_SEQ      ),
MKVOL(MP4_USER_DATA   ),
MKVOL(MP4_GOP         ),
MKVOL(MP4_VISUAL_OBJ  ),
MKVOL(MP4_VOP         )};

const char *findUnit(int c)
{
    int m=sizeof(descriptor)/sizeof(unitDesc);
    for(int i=0;i<m;i++)
        if(descriptor [i].unitId==c)
            return descriptor[i].unitName;
    return "unknown";
}
/**
    \fn splitMpeg4
    \brief split a bytestream block into mpeg4 sp/asp units
*/
int splitMpeg4(uint8_t *frame,uint32_t dataSize,mpeg4unit *unit,int maxUnits)
{
    uint8_t *start=frame;
    uint8_t *end=start+dataSize;
    int nbUnit=0;
    while(start+3<end)
    {
        uint8_t c;
        uint32_t offset;
        if(false==ADM_findMpegStartCode(start,end,&c,&offset)) break;
        ADM_assert(nbUnit<maxUnits);
#if 0
        printf("Unit : %x offset=%d absOffset=%d val=%x\n",c,offset,(int)(offset,start+offset-frame),
                                                            *(start+offset));
#endif
        ADM_assert(offset>=4);
        unit[nbUnit].code=c;
        unit[nbUnit].data=start+offset-4;
        unit[nbUnit].len=0;
        start=start+offset;
        nbUnit++;
    }
    //ADM_info("found %d units\n",nbUnit);
    if(!nbUnit) return 0;
    for(int j=0;j<nbUnit-1;j++)
    {
        unit[j].len=(uint32_t)(unit[j+1].data-unit[j].data);
    }
    unit[nbUnit-1].len=(uint32_t)(end-unit[nbUnit-1].data);
    /*for(int j=0;j<nbUnit;j++)
    {
        mpeg4unit *u=unit+j;
        ADM_info("%x : %s, offset=%d size=%d\n",u->code,findUnit(u->code),(int)(u->data-frame),u->len);
    }*/
    return nbUnit;
}
/**
    \fn extractVolHeader
    \brief extract VOL header from a frame, it will be used later as esds atom for example
*/
bool extractVolHeader(uint8_t *data,uint32_t dataSize,uint8_t **volStart, uint32_t *volLen)
{
    // Search startcode
    uint8_t b;
    uint32_t idx=0;
    uint32_t mw,mh;
    uint32_t time_inc;
    
    mpeg4unit unit[10];
    int nbUnit=splitMpeg4(data,dataSize,unit,10);
    if(!nbUnit)
    {
        ADM_error("Cannot find VOL header(1)\n");
        return false;
    }
    for(int i=0;i<nbUnit;i++)
    {
          mpeg4unit *u=unit+i;
          if(u->code==MP4_VOL)
          {
                ADM_info("Vol Header found : %x : %s, offset=%d size=%d\n",u->code,findUnit(u->code),
                                                        (int)(u->data-data),u->len);
                *volStart=u->data;
                *volLen=u->len;
                return true;
          }
    }
    ADM_error("Cannot find VOL header in the units\n");
    return false;
}    


//EOF
