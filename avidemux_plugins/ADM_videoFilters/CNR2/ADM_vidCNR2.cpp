//
//      chroma noise reduction II (version 2.6.1) - Avisynth filter reducing noise on chroma
//      Copyright (C) 2002 Marc Fauconneau
//
//      Inspired by :
//  chroma noise reduction (version 1.1) - VirtualDub filter reducing noise on chroma
//  Copyright (C) 2000 Gilles Mouchard
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  Please contact me for any bugs or questions.
//  marc.fd@libertysurf.fr
//
//  Change log :
//         30/06/2004 - ver 2.6.1 - YUY2 opts as I had slowed it down - tritical 
//         29/06/2004 - ver 2.6  - Some bug fixes and some code cleanup 
//                                 and rewriting (tritical - kes25c@mizzou.edu)
//         18/11/2003 - ver 2.51 - Further bug fixes (by Klaus Post)
//         13/11/2003 - ver 2.5  - Bug fixes (by Klaus Post)
//         15/12/2002 - ver 2.4  - Bug fixes
//         13/11/2002 - ver 2.3  - YV12 mode, scd (scenechange detection)
//         01/08/2002 - ver 2.2  - Ugly bug fixed
//         31/07/2002 - ver 2.1  - Bug Fixes (thx to dividee ;)
//         20/07/2002 - ver 2.0  - Avisynth filter coded (from scratch)

#include <math.h>
#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"

#include "ADM_vidCNR2_param.h"

//#define SUBST 1

class vidCNR2:public AVDMGenericVideoStream
{

protected:

  unsigned char *py, *py_saved, *cy, *cy_saved;
  unsigned char lt[513];
  unsigned char ut[513];
  unsigned char vt[513];
  int nfrms, keepTrack;
  unsigned int diffmax;

  virtual char *printConf (void);
  VideoCache *vidCache;
  CNR2Param *_param;
  void downSampleYV12 (unsigned char *dst, ADMImage * src);
  uint8_t setup (void);
public:

    vidCNR2 (AVDMGenericVideoStream * in, CONFcouple * setup);
    virtual ~ vidCNR2 ();
  virtual uint8_t getFrameNumberNoAlloc (uint32_t frame, uint32_t * len,
					 ADMImage * data, uint32_t * flags);
  uint8_t configure (AVDMGenericVideoStream * instream);
  virtual uint8_t getCoupledConf (CONFcouple ** couples);

};

static FILTER_PARAM cnr2_template =
  { 9, {"scdthr", "ln", "lm", "un", "um", "vn", "vm", "sceneChroma",
	"mode"} };

//REGISTERX(VF_NOISE, "cnr2",QT_TR_NOOP("Cnr2"),
//QT_TR_NOOP("Chroma noise reduction filter by MarcFD/Tritical."),VF_CNR2,1,cnr2_create,cnr2_script);
VF_DEFINE_FILTER_UI(vidCNR2,cnr2_template,
                cnr2,
                QT_TR_NOOP("Cnr2"),
                1,
                VF_NOISE,
                QT_TR_NOOP("Chroma noise reduction filter by MarcFD/Tritical.."));


extern uint8_t DIA_cnr2(CNR2Param *param);
/*************************************/
uint8_t vidCNR2::configure (AVDMGenericVideoStream * in)
{
        if( DIA_cnr2(_param))
        {
                setup();
                return 1;
        }
        return 0;
}
/*************************************/
char *vidCNR2::printConf (void)
{
  ADM_FILTER_DECLARE_CONF( " CNR2 by MarcFD/Tritical");
  
}
/*************************************/
vidCNR2::vidCNR2 (AVDMGenericVideoStream * in, CONFcouple * couples)
{

  _in = in;
  memcpy (&_info, _in->getInfo (), sizeof (_info));
  _info.encoding = 1;
  _param = NEW (CNR2Param);
  vidCache = new VideoCache (4, in);
  _uncompressed=new ADMImage(_info.width,_info.height);
  if (couples)
    {
#undef GET
#define GET(x) couples->getCouple(#x,&(_param->x))
      GET (scdthr);
      GET (ln);
      GET (lm);
      GET (un);
      GET (um);
      GET (vn);
      GET (vm);
      GET (sceneChroma);
      GET (mode);
    }
  else				// Default
    {
      _param->scdthr = 10.f;
      _param->lm = 192;
      _param->ln = 35;
      _param->un = 47;
      _param->um = 255;
      _param->vn = 47;
      _param->vm = 255;
      _param->sceneChroma = 0;
      _param->mode = 0x00FFFF; // u&v in narrow mode
    }
  //
  py_saved = py = cy_saved = cy = NULL;
  py = new unsigned char[(_info.width * _info.height) >> 2];
  py_saved = py;
  cy = new unsigned char[(_info.width * _info.height) >> 2];
  cy_saved = cy;
  nfrms = _info.nb_frames - 1;
  setup ();
}
/*************************************/
uint8_t vidCNR2::setup (void)
{
double root;
double num,denum,mul;

  root= _info.height * _info.width;
  root*=_param->scdthr;
  if (_param->sceneChroma)
    diffmax =      (int) ((root * 331.0f) / 100.0f);
  else
    diffmax =      (int) ((root* 219.0f) / 100.0f);

  memset (lt, 0, 513);		// for safety
  memset (ut, 0, 513);
  memset (vt, 0, 513);

  keepTrack = -39482;

  const double pi = M_PI;
  bool Y = true, U = true, V = true;
  if (_param->mode & 0xFF0000 )
    Y = false;
  if (_param->mode & 0x00FF00 )
    U = false;
  if (_param->mode & 0x0000FF )
    V = false;

// Reset
  int i, j;
  for (i = -256; i < 256; ++i)
  {
    lt[i + 256] = 0;
    ut[i + 256] = 0;
    vt[i + 256] = 0;
  }
 
 num=M_PI;
        /************************ Y********************/
 mul=_param->lm / 2.;
 if(Y)
 { 
  denum=_param->ln*_param->ln;
  for (j = -_param->ln; j <= _param->ln; ++j)
        lt[j + 256] =  (int) (mul * (1 + cos ((j * j * num) / denum)));
 }
 else
 {
  denum=_param->ln;
  for (j = -_param->ln; j <= _param->ln; ++j)
        lt[j + 256] =  (int) (mul * (1 + cos ((j * num) / denum)));
 
 }

  
    /************************ U ********************/
 mul=_param->um / 2.;
 if(U)
 {
        denum=_param->un*_param->un;
        for (j = -_param->un; j <= _param->un; ++j)
                ut[j + 256] =  (int) (mul * (1 + cos ((j * j * pi) / (denum))));
 }
 else
 {
        denum=_param->un;
        for (j = -_param->un; j <= _param->un; ++j)
                ut[j + 256] =  (int) (mul * (1 + cos ((j  * pi) / (denum))));

 }
      /************************ V ********************/  
  mul=_param->vm / 2.;
  if(V)
   {
        denum=_param->vn*_param->vn;
        for (j = -_param->vn; j <= _param->vn; ++j)
                vt[j + 256] =  (int) (mul * (1 + cos ((j * j * pi) / (denum))));
 }
 else
 {
        denum=_param->vn;
        for (j = -_param->vn; j <= _param->vn; ++j)
                vt[j + 256] =  (int) (mul * (1 + cos ((j  * pi) / (denum))));
 } 
  return 1;
}
//____________________________________________________________________
vidCNR2::~vidCNR2 ()
{

  delete _param;
  _param = NULL;
  DELETE( _param);
  delete vidCache;
  _param = NULL;
  vidCache = NULL;
  delete[]py_saved;
  delete[]cy_saved;
  
  py_saved = NULL;
  cy_saved = NULL;
  
  delete _uncompressed;
  _uncompressed=NULL;

}

//______________________________________________________________
uint8_t vidCNR2::getFrameNumberNoAlloc (uint32_t frame,
				uint32_t * len,
				ADMImage * data, uint32_t * flags)
{
  ADMImage *cur, *mprev, *src;

  if (frame >= _info.nb_frames)
    return 0;

  cur = vidCache->getImage (frame);
  src = cur;
  if (!frame)
    {
      data->duplicate (cur);
      vidCache->unlockAll ();
      return 1;
    }
  const unsigned char *srcpY = YPLANE (src);	//src->GetReadPtr(PLANAR_Y);
  const unsigned char *srcpU = UPLANE (src);	//src->GetReadPtr(PLANAR_U);
  const unsigned char *srcpV = VPLANE (src);	//src->GetReadPtr(PLANAR_V);
  const unsigned char *srcp;

  int src_pitchY = _info.width;	//src->GetPitch(PLANAR_Y);
  int src_pitchUV = _info.width >> 1;	//src->GetPitch(PLANAR_V);
  int heightY = _info.height;	//src->GetHeight(PLANAR_Y);
  int heightUV = _info.height >> 1;	//src->GetHeight(PLANAR_V);
  int widthY = _info.width ;	//src->GetRowSize(PLANAR_Y);
  int widthYd2 = widthY >> 1;
  int widthUV = _info.width >> 1;	//src->GetRowSize(PLANAR_V);

  downSampleYV12 (cy, src);
  if (keepTrack != frame)
    {
      mprev = vidCache->getImage (frame - 1);
      _uncompressed->duplicate(mprev); // not optimal  
      keepTrack = frame;
      downSampleYV12 (py, mprev);
    }
  unsigned char *dstpY = YPLANE (data);	//dst->GetWritePtr(PLANAR_Y);
  unsigned char *dstpU = UPLANE (data);	//dst->GetWritePtr(PLANAR_U);
  unsigned char *dstpV = VPLANE (data);	//dst->GetWritePtr(PLANAR_V);
  unsigned char *dstp, *prevy, *prevp, *curry, *t, *swap;
  int dst_pitchY = _info.width;	//dst->GetPitch(PLANAR_Y);
  int dst_pitchUV = _info.width >> 1;	//dst->GetPitch(PLANAR_V);
  int y, x, ydiff, uvdiff, cr;
  unsigned int difft = 0;
  const int off = 256;
  int res;
  // U plane (we add the luma plane diff to difft here not on v)
  prevy = py;
  curry = cy;
  t = ut;
  srcp = srcpU;
  dstp = dstpU;
  prevp = UPLANE (_uncompressed);	//prev->GetWritePtr(PLANAR_U);
  int prev_pitchUV = _info.width >> 1;	//prev->GetPitch(PLANAR_V);
  if (_param->sceneChroma)
    {
      for (y = 0; y < heightUV; ++y)
	{
	  for (x = 0; x < widthUV; ++x)
	    {
	      ydiff = curry[x] - prevy[x];
	      uvdiff = srcp[x] - prevp[x];
	      difft += abs (uvdiff) + abs (ydiff << 2);
	      cr = (lt[ydiff + off] * t[uvdiff + off]);
	       res=(cr * prevp[x] + (65536 - cr) * srcp[x] + 32768) >> 16;
#ifdef SUBST
                if(res!=srcp[x]) dstp[x]=120;
                        else dstp[x]=0;
#else
                 dstp[x] = prevp[x]=res;
#endif
	    }
	  if (difft > diffmax)
	    {
	      goto exit;
	    }
	  srcp += src_pitchUV;
	  dstp += dst_pitchUV;
	  prevp += prev_pitchUV;
	  curry += widthYd2;
	  prevy += widthYd2;
	}
    }
  else
    {
      for (y = 0; y < heightUV; ++y)
	{
	  for (x = 0; x < widthUV; ++x)
	    {
	      ydiff = curry[x] - prevy[x];
	      uvdiff = srcp[x] - prevp[x];
	      difft += abs (ydiff << 2);
	      cr = (lt[ydiff + off] * t[uvdiff + off]);
	      res =		(cr * prevp[x] + (65536 - cr) * srcp[x] + 32768) >> 16;
#ifdef SUBST
                if(res!=srcp[x]) dstp[x]=120;
                        else dstp[x]=0;
#else
                 dstp[x] = prevp[x]=res;
#endif

	    }
	  if (difft > diffmax)
	    {
	      goto exit;
	    }
	  srcp += src_pitchUV;
	  dstp += dst_pitchUV;
	  prevp += prev_pitchUV;
	  curry += widthYd2;
	  prevy += widthYd2;
	}
    }
  // V plane
  prevy = py;
  curry = cy;
  t = vt;
  srcp = srcpV;
  dstp = dstpV;
  prevp = VPLANE (_uncompressed);	//prev->GetWritePtr(PLANAR_V);
  if (_param->sceneChroma)
    {
      for (y = 0; y < heightUV; ++y)
	{
	  for (x = 0; x < widthUV; ++x)
	    {
	      ydiff = curry[x] - prevy[x];
	      uvdiff = srcp[x] - prevp[x];
	      difft += abs (uvdiff);
	      cr = (lt[ydiff + off] * t[uvdiff + off]);
	      res =(cr * prevp[x] + (65536 - cr) * srcp[x] + 32768) >> 16;
#ifdef SUBST
                if(res!=srcp[x]) dstp[x]=120;
                        else dstp[x]=0;
#else
                 dstp[x] = prevp[x]=res;
#endif

	    }
	  if (difft > diffmax)
	    {
	      goto exit;
	    }
	  srcp += src_pitchUV;
	  dstp += dst_pitchUV;
	  prevp += prev_pitchUV;
	  curry += widthYd2;
	  prevy += widthYd2;
	}
    }
  else
    {
      for (y = 0; y < heightUV; ++y)
	{
	  for (x = 0; x < widthUV; ++x)
	    {
	      ydiff = curry[x] - prevy[x];
	      uvdiff = srcp[x] - prevp[x];
	      cr = (lt[ydiff + off] * t[uvdiff + off]);
	      res =(cr * prevp[x] + (65536 - cr) * srcp[x] + 32768) >> 16;
#ifdef SUBST
                if(res!=srcp[x]) dstp[x]=120;
                        else dstp[x]=0;
#else
                 dstp[x] = prevp[x]=res;
#endif

	    }
	  srcp += src_pitchUV;
	  dstp += dst_pitchUV;
	  prevp += prev_pitchUV;
	  curry += widthYd2;
	  prevy += widthYd2;
	}
    }
exit:
//#define CNR_VERBOSE
#if defined( CNR_VERBOSE) && defined(ADM_DEBUG)
        printf("cur:%u max:%u\n",difft,diffmax);
#endif
        // Scene change ?
  if (difft > diffmax)
    {
      data->duplicate (cur);
      vidCache->unlockAll ();
      return 1;
    }
  ++keepTrack;
  // Dupe luma
  memcpy (dstpY, srcpY, _info.height * _info.width);

  swap = py;
  py = cy;
  cy = swap;
  vidCache->unlockAll ();
  return 1;
}

/*************************************/
void vidCNR2::downSampleYV12 (unsigned char *dst, ADMImage * src)
{
  unsigned char *temp = dst;
  const unsigned char *srcpY = YPLANE (src);
  int src_pitchY = _info.width << 1;
  const unsigned char *srcpnY = srcpY + (src_pitchY >> 1);
  int widthY = _info.width >> 1;
  int heightY = _info.height >> 1;
  int x, y, temp1;
  for (y = 0; y < heightY; ++y)
    {
      for (x = 0; x < widthY; ++x)
	{
	  temp1 = x << 1;
	  temp[x] =
	    (srcpY[temp1] + srcpY[temp1 + 1] + srcpnY[temp1] +
	     srcpnY[temp1 + 1] + 2) >> 2;
	}
      srcpY += src_pitchY;
      srcpnY += src_pitchY;
      temp += widthY;
    }
}
/*************************************/
uint8_t vidCNR2::getCoupledConf (CONFcouple ** couples)
{

  ADM_assert (_param);
  *couples = new CONFcouple (9);
#undef CSET
#define CSET(x)  (*couples)->setCouple(#x,(_param->x))

  CSET (scdthr);
  CSET (ln);
  CSET (lm);
  CSET (un);
  CSET (um);
  CSET (vn);
  CSET (vm);
  CSET (sceneChroma);
  CSET (mode);

  return 1;
}

// EOF
