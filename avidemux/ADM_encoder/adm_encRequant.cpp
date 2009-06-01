/***************************************************************************
           Requant encoder
    copyright            : (C) 2002/2006 by mean
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


#include "ADM_default.h"
#include "fourcc.h"
#include "avi_vars.h"



#include "ADM_encoder/ADM_vidEncode.hxx"

#include "ADM_videoFilter.h"
#include "ADM_encoder/adm_encoder.h"
#include "ADM_encoder/adm_encRequant.h"

extern "C"
{
#include "ADM_requant/tcrequant.h"
}

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_ENCODER
#include "ADM_osSupport/ADM_debug.h"
#define REQUANT_BUFFER (3*1024*1024)

static uint32_t searchForward (uint32_t startframe);
EncoderRequant::EncoderRequant (COMPRES_PARAMS * codecconfig)
{
  uint32_t p;
  float fp;

  _frameStart = _total = 0;
  _lastIPFrameSent = 0xFFFFFFFF;
  _buffer=new uint8_t[REQUANT_BUFFER];
  ADM_assert(codecconfig->extraSettingsLen==sizeof(uint32_t));
  p=*(uint32_t *)codecconfig->extraSettings;
  fp=p;
  fp/=1000; // 1000 > %100
  printf("Initializing requant with factor = %f\n",fp);
  Mrequant_init (fp,1);

}
EncoderRequant::~EncoderRequant ()
{
  if(_buffer) delete [] _buffer;
  _buffer=NULL;
  Mrequant_end();
}
uint8_t
EncoderRequant::isDualPass (void)
{
  return 0;
}
uint8_t
EncoderRequant::setLogFile (const char *p, uint32_t fr)
{
  return 1;
}
uint8_t
EncoderRequant::stop (void)
{
  return 1;
}
uint8_t
EncoderRequant::startPass2 (void)
{
  return 1;
}
uint8_t
EncoderRequant::startPass1 (void)
{
  return 1;
}
const char *
EncoderRequant::getDisplayName (void)
{
  return QT_TR_NOOP("Requant");
}


// that one is used as fourcc
const char *
EncoderRequant::getCodecName (void)
{
  aviInfo info;
  video_body->getVideoInfo (&info);
  return fourCC::tostring (info.fcc);
}
const char *
EncoderRequant::getFCCHandler (void)
{
  return "MPEG";
}

/****************************************************************/
uint8_t
EncoderRequant::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  aviInfo info;
  _in = instream;
  _frameStart = frameStart;
  video_body->getVideoInfo (&info);
  if (info.nb_frames == frameEnd + 1)	// last frame included ?
    {
      _total = _total = frameEnd - frameStart + 1;
    }
  else
    _total = frameEnd - frameStart;

  uint32_t end, flags;
next:
  end = frameStart + _total - 1;
  video_body->getFlags (end, &flags);
  if ((flags & AVI_B_FRAME) && _total)
    {
      _total--;
      goto next;
    }

  return 1;
}
/**

*/
uint8_t EncoderRequant::code(uint32_t frame,uint8_t *out, uint32_t *outlen)
{
  uint32_t len;
  uint8_t ret,seq;
  ADMCompressedImage image;
    image.data=_buffer;
    image.dataLength=REQUANT_BUFFER;
  ret = video_body->getFrame(frame,&image,&seq);
  if(!ret)
  {
    printf("[Requant] Cannot read frame %u\n",frame);
    return 0;
  }
  Mrequant_frame(_buffer,  len,out, outlen);
  return 1;
}

/****************************************************************/
uint8_t
EncoderRequant::encode (uint32_t frame, ADMBitstream *out)
{
  uint8_t ret = 0;
//  out->dtsFrame = frame;

  if (frame >= _total)
    {
      printf ("[Requant]: Going out of bound %d/%d\n", frame, _total);
      // Stuff with emptyness
      out->len=0;
      return 1;
    }
    out->flags=0;
    /* First frame ? */

  // No B frames, take as is
  if (!video_body->isReordered (frameStart + frame))
    {

          ret =code(_frameStart + frame, out->data, &out->len);
//          out->ptsFrame = frame;
          return ret;
    }
  // it has PTS/DTS stuff so we need to reorder it

    video_body->getFlags (frameStart + frame, &(out->flags));
  if (out->flags & AVI_B_FRAME)
    {				// search if we have to send a I/P frame in adance
      aprintf ("\tIt is a B frame\n");
      uint32_t forward;

      forward = searchForward (_frameStart + frame);
      // if we did not sent it, do it now
      if (forward != _lastIPFrameSent)
      {
          aprintf ("\tP Frame not sent, sending it :%lu\n", forward);
          ret =code(forward, out->data, &out->len);
          //ret = video_body->getRaw (forward, out->data, &out->len);
          _lastIPFrameSent = forward;
//          out->ptsFrame = forward - _frameStart;
        }
    else
        {
        // we already sent it :)
        // send n-1
            aprintf ("\tP Frame already, sending  :%lu\n",
                frameStart + frame - 1);
            ret =code(_frameStart + frame - 1, out->data, &out->len);
//             ret =video_body->getFrameNoAlloc (_frameStart + frame - 1, out->data,
//                                               &out->len,&out->flags);
//            out->ptsFrame = frame - 1;
        }
    }
  else	     // it is not a B frame and we have nothing on hold, sent it..
    {
      // send n-1 if we reach the fwd reference frame
      if ((frame + frameStart) == _lastIPFrameSent)
        {
            aprintf ("\tSending Last B-frame :(%lu)\n",
                    _frameStart + frame - 1);
            //ret=video_body->getFrameNoAlloc (_frameStart+frame - 1,out->data,&out->len,&out->flags);
            ret =code(_frameStart + frame - 1, out->data, &out->len);
//            out->ptsFrame  = frame - 1;

        }
      else
	{
            aprintf ("\tJust sending it :(%lu)-(%lu)\n", _frameStart + frame,
                    _lastIPFrameSent);
            ret =code(_frameStart + frame , out->data, &out->len);
              //  ret=video_body->getFrameNoAlloc (_frameStart+frame, out->data,&out->len,&out->flags);
//            out->ptsFrame  = frame;
            if(!frame) // First frame ?
            {
                  // It does not start by a seqstart, add it if possible
                  if(!(_buffer[0]==0 && _buffer[1]==0 && _buffer[2]==1 && _buffer[3]==0xb3))
                  {
                    uint8_t buf[10*1024];
                    uint8_t seq;
                    ADMCompressedImage image;
                    image.data=buf;
                    image.dataLength=REQUANT_BUFFER;
                    video_body->getFrame (frameStart,&image, &seq);
                    printf("Adding seq header (%"LU")\n",seq);
                    memmove(_buffer+seq,_buffer,out->len);
                    memcpy(_buffer,buf,seq);
                    out->len+=seq;

                  }
            }
        }
    }
  if (!ret)
    printf ("Get frame error for frame %d+%d\n", frameStart, frame);
  return ret;
}

uint8_t
EncoderRequant::hasExtraHeaderData (uint32_t * l, uint8_t ** data)
{

  *l=0;
  *data=NULL;
  return 1;//video_body->getExtraHeaderData (l, data);
}

uint32_t
searchForward (uint32_t startframe)
{
  uint32_t fw = startframe;
  uint32_t flags;
  uint8_t r;

  while (1)
    {
      fw++;
      r = video_body->getFlags (fw, &flags);
      if (!(flags & AVI_B_FRAME))
	{
	  return fw;

	}
      ADM_assert (r);
      if (!r)
	{
	  printf ("\n Could not locate last non B frame \n");
	  return 0;
	}

    }
}
