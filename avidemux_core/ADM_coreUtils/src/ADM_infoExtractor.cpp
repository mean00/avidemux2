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
//#include "ADM_mp4.h"

#define aprintf(...) {}
#include "ADM_getbits.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"

bool ADM_findMpegStartCode (uint8_t * start, uint8_t * end,
			    uint8_t * outstartcode, uint32_t * offset);
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
	  if ((data[idx] & 0xF0) == 0x20)	//VOL start
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
	      GetBitContext s;
	      init_get_bits (&s, data + idx, dataSize * 8);
	      //
	      skip_bits1 (&s);	// Random access
	      skip_bits (&s, 8);	// Obj type indication
	      if (get_bits (&s, 1))	// VO od 
		{
		  skip_bits (&s, 4);	// Ver
		  skip_bits (&s, 3);	// Priority
		}
	      if (get_bits (&s, 4) == 15)	// custom A/R
		{
		  skip_bits (&s, 8);
		  skip_bits (&s, 8);
		}
	      if (get_bits (&s, 1))	// Vol control param
		{
		  skip_bits (&s, 2);	//Chroma
		  skip_bits (&s, 1);	// Low delay
		  if (get_bits (&s, 1))	// VBV Info
		    {
		      skip_bits (&s, 16);
		      skip_bits (&s, 16);
		      skip_bits (&s, 16);
		      skip_bits (&s, 15);
		      skip_bits (&s, 16);
		    }
		}
	      skip_bits (&s, 2);	//  Shape
	      skip_bits (&s, 1);	//  Marker
	      timeVal = get_bits (&s, 16);	// Time increment
	      *time_inc = av_log2 (timeVal - 1) + 1;
	      if (*time_inc < 1)
		*time_inc = 1;
	      skip_bits (&s, 1);	//  Marker
	      if (get_bits (&s, 1))	// Fixed vop rate, compute how much bits needed
		{
		  get_bits (&s, *time_inc);
		}
	      skip_bits (&s, 1);	//  Marker
	      mw = get_bits (&s, 13);
	      skip_bits (&s, 1);	//  Marker
	      mh = get_bits (&s, 13);
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
  GetBitContext s;
  int vop;
  uint32_t vp, tinc;
  init_get_bits (&s, data, len * 8);
  vop = get_bits (&s, 2);
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
  while (get_bits1 (&s) != 0)
    imodulo++;
  if (!get_bits1 (&s))
    {
      printf ("Wrong marker1\n");
      return 0;
    }

  /* Read time */
  tinc = get_bits (&s, timeincbits);
  /* Marker */
  if (!get_bits1 (&s))
    {
      printf ("Wrong marker2\n");
      return 0;
    }
  /* Vop coded */
  *modulo = imodulo;
  *vopcoded = get_bits1 (&s);
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
  GetBitContext gb;
  int format;
  init_get_bits (&gb, buffer, len * 8);
  if (get_bits_long (&gb, 17) != 1)
    {
      printf ("[FLV]Wrong FLV1 header\n");
      return 0;
    }
  format = get_bits (&gb, 5);
  if (format != 0 && format != 1)
    {
      printf ("[FLV]Wrong FLV1 header format\n");
      return 0;
    }

  get_bits (&gb, 8);		/* picture timestamp */
  format = get_bits (&gb, 3);
  switch (format)
    {
    case 0:
      *w = get_bits (&gb, 8);
      *h = get_bits (&gb, 8);
      break;
    case 1:
      *w = get_bits (&gb, 16);
      *h = get_bits (&gb, 16);
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
/*
        Extract H263 width & height from header

*/
uint8_t
extractH263Info (uint8_t * data, uint32_t dataSize, uint32_t * w,
		 uint32_t * h)
{
  uint32_t val;
  GetBitContext s;
  init_get_bits (&s, data, dataSize * 8);

  mixDump (data, 10);
  val = get_bits (&s, 16);
  if (val)
    {
      printf ("incorrect H263 header sync\n");
      return 0;
    }
  val = get_bits (&s, 6);
  if (val != 0x20)
    {
      printf ("incorrect H263 header sync (2)\n");
      return 0;
    }
  //
  skip_bits (&s, 8);		// timestamps in 30 fps tick
  skip_bits (&s, 1);		// Marker
  skip_bits (&s, 1);		// Id
  skip_bits (&s, 1);		// Split
  skip_bits (&s, 1);		// Document Camera indicator
  skip_bits (&s, 1);		// Full Picture Freeze Release
  val = get_bits (&s, 3);
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
	  if (code == 0xb6)
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


//EOF
