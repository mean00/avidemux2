
/***************************************************************************
                         
        Tdeint

    copyright            : Tritical, ported to avidemux by mean
   http://bengal.missouri.edu/~kes25c/
 ***************************************************************************/
/*
**                TDeinterlace v1.0b4 for AviSynth 2.5.x
**
**   TDeinterlace is a bi-directionally motion adaptive deinterlacer.
**   It also uses a couple modified forms of ela interpolation which 
**   help to reduce "jaggy" edges in places where interpolation must 
**   be used. TDeinterlace currently supports YV12 and YUY2 colorspaces.
**   
**   Copyright (C) 2004-2005 Kevin Stone
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   This program is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include <math.h>

#include "DIA_factory.h"
#define aprintf(...) {}

#define min MIN
#define max MAX

#define MIN(a,b) a<b ? a : b
#define MAX(a,b) b<a ? a : b
#define child_GetParity(x) 0 //FIXME!!

#define OutputDebugString(img,y,text) drawString(img, 0, y, text)


#include "ADM_vidTDeint_param.h"
#define PRM(x) x
class vidTDeint:public AVDMGenericVideoStream
{

protected:
virtual char          *printConf (void);
VideoCache            *vidCache;
ADMImage              *rebuild,*scratch,*scratch2;
TDEINT_PARAM          *_param;
/* Will go to param */
int mode, order, field, ovrDefault, type, mtnmode;
	int mthreshL, mthreshC, map, cthresh, MI, link;
	int countOvr, nfrms, nfrms2, orderS, fieldS;
	int mthreshLS, mthreshCS, typeS, cthresh6, AP;
	int xhalf, yhalf, xshift, yshift, blockx, blocky;
	int mntmode;
	
	int *input, *cArray, APType;
	unsigned int passHint;
	unsigned long accumN, accumP;
	bool debug, sharp, hints, full, chroma;
	bool autoFO, useClip2, tryWeave, denoise;
	const char* ovr;
	char buf[120];
/* Will go to param */
void  copyFrame(ADMImage *dst,ADMImage *src);

unsigned char         cubicInt(unsigned char p1, unsigned char p2, unsigned char p3,unsigned char p4);
void  copyForUpsize(ADMImage *dst, ADMImage *src, int np);
void  setMaskForUpsize(ADMImage *msk, int np);
void  createMotionMapYV12(ADMImage *prv2, ADMImage *prv, 
	                       ADMImage *src, ADMImage *nxt, ADMImage *nxt2, ADMImage *mask, int n);
void  createMotionMap2YV12(ADMImage *prv2, ADMImage *prv, 
	                       ADMImage *src, ADMImage *nxt, ADMImage *nxt2, ADMImage *mask, int n)	;
void  linkFULL_YV12(ADMImage *mask) ;	
void  linkYtoUV_YV12(ADMImage *mask)  ;                                               
void  linkUVtoY_YV12(ADMImage *mask) ;
void  denoiseYV12(ADMImage *mask) ;
bool  checkCombedYV12(ADMImage *src) ;
void  subtractFieldsYV12(ADMImage *prv, ADMImage *src, ADMImage *nxt) ;
void  mapColorsYV12(ADMImage *dst, ADMImage *mask);
void  mapMergeYV12(ADMImage *dst, ADMImage *mask, 
		ADMImage *prv, ADMImage *src, ADMImage *nxt);
void  cubicDeintYV12(ADMImage *dst, ADMImage *mask, 
		ADMImage *prv, ADMImage *src, ADMImage *nxt);
void  ELADeintYV12(ADMImage *dst, ADMImage *mask, 
		ADMImage *prv, ADMImage *src, ADMImage *nxt);
void  kernelDeintYV12(ADMImage *dst, ADMImage *mask, 
		ADMImage *prv, ADMImage *src, ADMImage *nxt);
void  smartELADeintYV12(ADMImage *dst, ADMImage *mask, 
		ADMImage *prv, ADMImage *src, ADMImage *nxt);
void  createWeaveFrameYV12(ADMImage *dst, ADMImage *prv, 
		ADMImage *src, ADMImage *nxt);
int  getHint(ADMImage *src, unsigned int &storeHint, int &hintField);
void putHint(ADMImage *src, unsigned int hint, int fieldt);
void apPostCheck(ADMImage *dst, ADMImage *mask);
void reset(void);
public:

                        vidTDeint (AVDMGenericVideoStream * in, CONFcouple * setup);
        virtual         ~vidTDeint ();
virtual uint8_t getFrameNumberNoAlloc (uint32_t frame, uint32_t * len,
                                        ADMImage * data, uint32_t * flags);
uint8_t configure (AVDMGenericVideoStream * instream);
virtual uint8_t getCoupledConf (CONFcouple ** couples);

};

static FILTER_PARAM tdeint_template =
  { 21,
"mode",
"order",
"field",
"mthreshL",
"mthreshC",
"map",
"type",
"debug",
"mtnmode",
"sharp",
"full",
"cthresh",
"blockx",
"blocky",
"chroma",
"MI",
"tryWeave",
"link",
"denoise",
"AP",
"APType"

  };

//********** Register chunk ************



VF_DEFINE_FILTER(vidTDeint,tdeint_template,
    tdeint,
                                QT_TR_NOOP("TDeint"),
                                1,
                                VF_INTERLACING,
                                QT_TR_NOOP("Motion adaptative deinterlacer by Tritical."));
//************************************

#include "ADM_vidTdeint_util.txt"
extern uint8_t  DIA_tdeint(TDEINT_PARAM *param);
//*************************************
uint8_t vidTDeint::configure (AVDMGenericVideoStream * in)
{
uint8_t r;
        r= DIA_tdeint(_param);
        if(r) reset();
        return r;
}
/*************************************/
char *vidTDeint::printConf (void)
{
 ADM_FILTER_DECLARE_CONF( " Tritical TDeint");
  
}

#define MAX_BLOCKS 50
/*************************************/
vidTDeint::vidTDeint (AVDMGenericVideoStream * in, CONFcouple * couples)
{

  _in = in;
  memcpy (&_info, _in->getInfo (), sizeof (_info));
  _info.encoding = 1;
  vidCache = new VideoCache (10, in);
  _uncompressed=new ADMImage(_info.width,_info.height);
  scratch=new ADMImage(_info.width,_info.height);
  scratch2=new ADMImage(_info.width,_info.height);

  	input = cArray = NULL;
  _param=new TDEINT_PARAM;
  if(!couples)
  {
  
                _param->mode=0;
                _param->order=-1;
                _param->field=-1;
                _param->mthreshL=6;
                _param->mthreshC=6;
                _param->map=0;
                _param->type=2;
                _param->debug=0;
                _param->mtnmode=1;
                _param->sharp=1;
                _param->full=1;
                _param->cthresh=6;
                _param->blockx=16;
                _param->blocky=16;
                _param->chroma=0;
                _param->MI=64;
                _param->tryWeave=false;
                _param->link=2;
                _param->denoise=true;
                _param->AP=254;
                _param->APType=1;
  }
  else
  {
                GET(mode);
                GET(order);
                GET(field);
                GET(mthreshL);
                GET(mthreshC);
                GET(map);
                GET(type);
                GET(debug);
                GET(mtnmode);
                GET(sharp);
                GET(full);
                GET(cthresh);
                GET(blockx);
                GET(blocky);
                GET(chroma);
                GET(MI);
                GET(tryWeave);
                GET(link);
                GET(denoise);
                GET(AP);
                GET(APType);

  
  
  }

    order=1;
    orderS=1;
    mode=0;
    field=0;
    fieldS=0;
    mthreshL=6;
    mthreshLS=6;
    mthreshC=6;
    mthreshCS=6;
    map=0;
    ovrDefault=0;
    type=2; // kernel deint
    debug=true;
    mntmode=1;
    mtnmode=1;
    sharp=true;
    hints=false;
    full=false;
    cthresh=12;
    ovr=NULL;
    input=NULL;
    blocky=16;
    blockx=16;
    chroma=false;
    MI=64;
    tryWeave=false;
    link=2;
    AP=254;
    APType=254;
    
    reset();

}
void vidTDeint::reset (void)
{

#define CLONE(x) x=_param->x
CLONE(mode);
CLONE(order);
CLONE(field);
CLONE(mthreshL);
CLONE(mthreshC);
CLONE(map);
CLONE(type);
CLONE(debug);
CLONE(mtnmode);
CLONE(sharp);
CLONE(full);
CLONE(cthresh);
CLONE(blockx);
CLONE(blocky);
CLONE(chroma);
CLONE(MI);
CLONE(tryWeave);
CLONE(link);
CLONE(denoise);
CLONE(AP);
CLONE(APType);

orderS=order;
fieldS=field;
mthreshLS=  mthreshL;
mthreshCS=mthreshC;


         xhalf = blockx >> 1;
        yhalf = blocky >> 1;
        xshift = blockx == 4 ? 2 : blockx == 8 ? 3 : blockx == 16 ? 4 : blockx == 32 ? 5 :
                blockx == 64 ? 6 : blockx == 128 ? 7 : blockx == 256 ? 8 : blockx == 512 ? 9 : 
                blockx == 1024 ? 10 : 11;
        yshift = blocky == 4 ? 2 : blocky == 8 ? 3 : blocky == 16 ? 4 : blocky == 32 ? 5 :
                blocky == 64 ? 6 : blocky == 128 ? 7 : blocky == 256 ? 8 : blocky == 512 ? 9 : 
                blocky == 1024 ? 10 : 11;
        if (((!full && mode == 0) || tryWeave) && mode >= 0)
        {
        int sz;
        
        sz=(((_info.width+xhalf)>>xshift)+1)*(((_info.height+yhalf)>>yshift)+1)*4;
                if(cArray) delete [] cArray;
                cArray = new int[sz];;
                
        }
        
        nfrms = nfrms2 = _info.nb_frames - 1;
        accumP = accumN = 0;
        cthresh6 = cthresh * 6;
        passHint = 0xFFFFFFFF;
        autoFO = false;
        if (order == -1) autoFO = true;
#if 0
        if (mode < 0) 
        {
                _info.height *= 2;
                field = 1;
        }
        
        if (mode == 1)
        {
                vi.num_frames *= 2;
                nfrms2 = vi.num_frames - 1;
                vi.SetFPS(vi.fps_numerator*2, vi.fps_denominator);
        }
        else
#endif   
        if (field == -1 && mode!=1)
        {
                // telecide matches off the bottom field so we want field=0 in that case.
                // tfm can match off top or bottom, but it will indicate which in its hints
                // and field is adjusted appropriately then... so we use field=0 by default
                // if hints=true.  Otherwise, if hints=false, we default to field = 1.
                if (hints) field = 0;
                else field = 1;
        }
        orderS = order; 
        fieldS = field; 
        mthreshLS = mthreshL; 
        mthreshCS = mthreshC;
        typeS = type;
#if 0
        if (debug)
        {
                sprintf(buf,"TDeint:  %s (%s) by tritical\n", "B4", "08 2005");
                OutputDebugString(buf);
                sprintf(buf,"TDeint:  mode = %d (%s)\n", mode, mode == 0 ? "normal - same rate" : 
                                mode == 1 ? "bob - double rate" : mode == -2 ? "upsize - ELA" : "upsize - ELA-2");
                OutputDebugString(buf);
        }
#endif
}
//____________________________________________________________________
vidTDeint::~vidTDeint ()
{

  delete vidCache;
  vidCache = NULL;
  delete _uncompressed;
  _uncompressed=NULL;
  delete scratch;
  scratch=NULL;
  delete scratch2;
  scratch2=NULL;
  if(cArray) delete [] cArray;
  cArray=NULL;
}






uint8_t vidTDeint::getCoupledConf (CONFcouple ** couples)
{

  ADM_assert (_param);
  *couples = new CONFcouple (21);
#undef CSET
#define CSET(x)  (*couples)->setCouple(#x,(_param->x))
CSET(mode);
CSET(order);
CSET(field);
CSET(mthreshL);
CSET(mthreshC);
CSET(map);
CSET(type);
CSET(debug);
CSET(mtnmode);
CSET(sharp);
CSET(full);
CSET(cthresh);
CSET(blockx);
CSET(blocky);
CSET(chroma);
CSET(MI);
CSET(tryWeave);
CSET(link);
CSET(denoise);
CSET(AP);
CSET(APType);

  return 1;
}

//***************************************************
uint8_t vidTDeint::getFrameNumberNoAlloc (uint32_t n,
                                uint32_t * len,
                                ADMImage * data, uint32_t * flags)
{

	int nfrms=_info.nb_frames;
	ADMImage *srcP,*srcN,*src,*final,*display;
        float distMerged, distN,distP,distM,distR,skip=0;
        char txt[255];

        if(n>= _info.nb_frames) return 0;
        if(n<1 || n>_info.nb_frames-3 )
        {
                skip=1;
        }
        
        
        if(skip)
        {
                data->duplicate(vidCache->getImage(n));
                vidCache->unlockAll();
                return 1;
        }
      
	if (mode < 0)
	{
		//PVideoFrame src2up = child->GetFrame(n, env);
		//mask2up=new ADMImage(_info.width,_info.height);
		ADMImage *src2up;
		src2up=vidCache->getImage(n);
		ADMImage *msk2up=scratch;
		if(!src2up) return 0;
		//PVideoFrame dst2up = env->NewVideoFrame(vi);
		//PVideoFrame msk2up = env->NewVideoFrame(vi);
		copyForUpsize(data, src2up, 3);
		setMaskForUpsize(msk2up, 3);
		if (mode == -2) smartELADeintYV12(data, msk2up, data, data, data);
		else if (mode == -1) ELADeintYV12(data, msk2up, data, data, data);
		//return dst2up;
		//delete mask2up;
		vidCache->unlockAll();
		return 1;
	}
	if (mode == 1)
	{
		if (autoFO) PRM(order) = child_GetParity(n>>1) ? 1 : 0;
		if (n&1) PRM(field) = PRM(order) == 1 ? 0 : 1;
		else PRM(field) = PRM(order);
		n >>= 1;
	}
	else if (autoFO) PRM(order) = child_GetParity(n) ? 1 : 0;
	//PVideoFrame prv2, prv, nxt, nxt2, dst, mask;
	//PVideoFrame src = child->GetFrame(n, env);
	src=vidCache->getImage(n);
	if(!src)
	{
        vidCache->unlockAll();
        return 0;	
	}
	ADMImage *prv2,*prv,*nxt,*nxt2,*dst,*mask;
	bool found = false, fieldOVR = false;
	int x, hintField = -1;
	passHint = 0xFFFFFFFF;
	if (input != NULL && *ovr)
	{
		if (mode != 1) 
		{
			PRM(field) = PRM(fieldS); 
			if (!autoFO) PRM(order) = PRM(orderS);
		}
		PRM(mthreshL) = PRM(mthreshLS);
		mthreshC = mthreshCS;
		type = typeS;
		for (x=0; x<countOvr; x+=4)
		{
			if (n >= input[x+1] && n <= input[x+2])
			{
				if (input[x] == 45 && mode != 1) // -
				{
					if (debug)
					{
						sprintf(buf,"TD fr %d:  not deinterlacing\n", n);
						OutputDebugString(data,0,buf);
					}
					data->duplicate(src);
					vidCache->unlockAll();
					return 1;
					//return src;
				}
				else if (input[x] == 43 && mode != 1) found = true;  // +
				else if (input[x] == 102 && mode != 1) { field = input[x+3]; fieldOVR = true; } // f
				else if (input[x] == 111 && mode != 1) PRM(order) = input[x+3]; // o
				else if (input[x] == 108) mthreshL = input[x+3]; // l
				else if (input[x] == 99) mthreshC = input[x+3]; // c
				else if (input[x] == 116) type = input[x+3]; // t
			}
		}
		if (!found && ovrDefault == 1 && mode != 1)
		{
			if (debug)
			{
				sprintf(buf,"TD fr %d:  not deinterlacing\n", n);
				OutputDebugString(data,0,buf);
			}
			data->duplicate(src);
			vidCache->unlockAll();
			return 1;
			//return src;
		}
	}
	if (mode == 0 && hints && vidTDeint::getHint(src, passHint, hintField) == 0 && !found) 
	{
		if (debug)
		{
			sprintf(buf,"TD fr %d:  not deinterlacing (HINTS)\n", n);
			OutputDebugString(data,0,buf);
		}
		data->duplicate(src);
			vidCache->unlockAll();
			return 1;
			//return src;
	}
	if (mode == 0 && !full && !found)
	{
		if (!checkCombedYV12(src))
		{
			if (debug)
			{
				sprintf(buf,"TD fr %d:  not deinterlacing (full = false)\n", n);
				OutputDebugString(data,0,buf);
			}
			data->duplicate(src);
			vidCache->unlockAll();
			return 1;
			//return src;
		}
	}
	if (!fieldOVR && hintField >= 0)
	{
		int tempf = field;
		field = hintField;
		hintField = tempf;
	}
	//if (!useClip2)
	{
		prv2 = vidCache->getImage(n>1 ? n-2 : n>0 ? n-1 : 0);//child->GetFrame(n>1 ? n-2 : n>0 ? n-1 : 0, env);
		prv = vidCache->getImage(n>0 ? n-1 : 0); //child->GetFrame(n>0 ? n-1 : 0, env);
		nxt = vidCache->getImage(n<nfrms ? n+1: nfrms); //child->GetFrame(n<nfrms ? n+1 : nfrms, env);
		nxt2 = vidCache->getImage(n<nfrms-1 ? n+2 : n<nfrms ? n+1 : nfrms); //child->GetFrame(n<nfrms-1 ? n+2 : n<nfrms ? n+1 : nfrms, env);
	}
	
	//dst = env->NewVideoFrame(vi);
	dst=data;
	if (type == 2 || mtnmode > 1 || tryWeave) 
	{
		subtractFieldsYV12(prv, src, nxt);
		if (debug)
		{
			sprintf(buf, "TD fr %d:  accumP = %u  ", n, accumP);
			OutputDebugString(data,2,buf);
			sprintf(buf, "accumN = %u\n", accumN);
			OutputDebugString(data,3,buf);
		}
	}
	if (tryWeave && (mode != 0 || full || found || (field^PRM(order) && accumP > accumN) || 
			(!(field^PRM(order)) && accumN > accumP)))
	{
		createWeaveFrameYV12(dst, prv, src, nxt);
		if (!checkCombedYV12(dst))
		{
			if (debug)
			{
				sprintf(buf,"TD  fr %d:  weaved with %s (tryWeave)\n", n, 
					field^PRM(order) ? (accumP <= accumN ? "CURR" : "NEXT") : 
					(accumN <= accumP ? "CURR" : "PREV"));
				OutputDebugString(data,2,buf);
			}
			if (hintField >= 0 && !fieldOVR) field = hintField;
			vidCache->unlockAll();
			return 1;//dst;
		}
	}
	mask =scratch2 ; // env->NewVideoFrame(vi);
	if (mthreshL <= 0 && mthreshC <= 0) setMaskForUpsize(mask, 3);
	else if (mtnmode == 0 || mtnmode == 2) createMotionMapYV12(prv2, prv, src, nxt, nxt2, mask, n);
	else if (mtnmode == 1 || mtnmode == 3) createMotionMap2YV12(prv2, prv, src, nxt, nxt2, mask, n);
	else {ADM_assert(0);}
	if (denoise) denoiseYV12(mask);
	if (link == 1) linkFULL_YV12(mask);
	else if (link == 2) linkYtoUV_YV12(mask);
	else if (link == 3) linkUVtoY_YV12(mask);
	else if (link != 0) {ADM_assert(0);}//env->ThrowError("TDeint:  an unknown error occured (link)!");
	if (map == 1) mapColorsYV12(dst, mask);
	else if (map == 2) mapMergeYV12(dst, mask, prv, src, nxt);
	else if (type == 0) cubicDeintYV12(dst, mask, prv, src, nxt);
	else if (type == 1) smartELADeintYV12(dst, mask, prv, src, nxt);
	else if (type == 2) kernelDeintYV12(dst, mask, prv, src, nxt);
	else if (type == 3) ELADeintYV12(dst, mask, prv, src, nxt);
	else {ADM_assert(0);}//env->ThrowError("TDeint:  an unknown error occured!");
	if (AP >= 0 && AP < 255 && map == 0) apPostCheck(dst, mask);
	if (!(passHint&0xFFFFFF00)) vidTDeint::putHint(dst, passHint, field);
	if (debug)
	{
		sprintf(buf,"TD  fr %d:  field = %s  order = %s\n", n, 
			field == 1 ? "bottom" : "top", PRM(order) == 1 ? "tff" : "bff");
		OutputDebugString(data,3,buf);
		sprintf(buf,"TD  fr %d:  mthreshL = %d  \n",n,mthreshL);
		OutputDebugString(data,4,buf);
		sprintf(buf,"mthreshC = %d  type = %d\n", mthreshC, type);
		OutputDebugString(data,5,buf);
	}
	if (hintField >= 0 && !fieldOVR) field = hintField;
	vidCache->unlockAll();
	return 1;
	//return dst;
}

//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************

#if 0
AVSValue __cdecl Create_TDeinterlace(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	int mode = 0;
	int order = -1;
	int field = -1;
	int mthreshL = 6;
	int mthreshC = 6;
	int map = 0;
	char* ovr = "";
	int ovrDefault = 0;
	int type = 2;
	bool debug = false;
	int mtnmode = 1;
	bool sharp = true;
	bool hints = false;
	bool full = true;
	int cthresh = 6;
	bool chroma = false;
	int MI = 64;
	bool tryWeave = false;
	int link = 2;
	bool denoise = true;
	int AP = -1;
	int blockx = 16, blocky = 16;
	int APType = 1;
	if (args[0].IsClip())
	{
		unsigned int temp;
		int tfieldHint;
		if (!args[13].IsBool() &&
			TDeinterlace::getHint(args[0].AsClip()->GetFrame(0,env), temp, tfieldHint) != -1)
			hints = true;
	}
	PClip v;
	if (args[14].IsClip())
	{
		v = args[14].AsClip();
		try
		{ 
			v = env->Invoke("InternalCache", v).AsClip();
			v->SetCacheHints(CACHE_RANGE, 4);
		} 
		catch (IScriptEnvironment::NotFound) {  }
	}
    return new TDeinterlace(args[0].AsClip(),args[1].AsInt(mode),args[2].AsInt(order),
		args[3].AsInt(field),args[4].AsInt(mthreshL),args[5].AsInt(mthreshC),args[6].AsInt(map),
		args[7].AsString(ovr),args[8].AsInt(ovrDefault),args[9].AsInt(type),args[10].AsBool(debug),
		args[11].AsInt(mtnmode),args[12].AsBool(sharp),args[13].AsBool(hints),args[14].IsClip() ? v : NULL,
		args[15].AsBool(full),args[16].AsInt(cthresh),args[17].AsBool(chroma),args[18].AsInt(MI),
		args[19].AsBool(tryWeave),args[20].AsInt(link),args[21].AsBool(denoise),args[22].AsInt(AP),
		args[23].AsInt(blockx),args[24].AsInt(blocky),args[25].AsInt(APType),env);
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) 
{
    env->AddFunction("TDeint", "c[mode]i[order]i[field]i[mthreshL]i[mthreshC]i[map]i[ovr]s[ovrDefault]i" \
		                       "[type]i[debug]b[mtnmode]i[sharp]b[hints]b[clip2]c[full]b[cthresh]i" \
							   "[chroma]b[MI]i[tryWeave]b[link]i[denoise]b[AP]i[blockx]i[blocky]i[APType]i", 
							   Create_TDeinterlace, 0);
	return 0;
}
TDeinterlace::TDeinterlace(PClip _child, int _mode, int _order, int _field, int _mthreshL, 
	int _mthreshC, int _map, const char* _ovr, int _ovrDefault, int _type, bool _debug, 
	int _mtnmode, bool _sharp, bool _hints, PClip _clip2, bool _full, int _cthresh, 
	bool _chroma, int _MI, bool _tryWeave, int _link, bool _denoise, int _AP, 
	int _blockx, int _blocky, int _APType, IScriptEnvironment* env) :
GenericVideoFilter(_child), mode(_mode), order(_order), field(_field), mthreshL(_mthreshL), 
	mthreshC(_mthreshC), map(_map), ovr(_ovr), ovrDefault(_ovrDefault), type(_type), 
	debug(_debug), mtnmode(_mtnmode), sharp(_sharp), hints(_hints), clip2(_clip2), full(_full),
	cthresh(_cthresh), chroma(_chroma), MI(_MI), tryWeave(_tryWeave), link(_link), 
	denoise(_denoise), AP(_AP), blockx(_blockx), blocky(_blocky), APType(_APType)
{
	int z, w, q, b, i, track, count;
	char linein[81];
	char *linep;
	FILE *f = NULL;         ***** order mode field map type mntmode 
	input = cArray = NULL;
	if (!vi.IsYV12() && !vi.IsYUY2()) 
		env->ThrowError("TDeint:  YV12 and YUY2 data only!");
	if (mode != 0 && mode != 1 && mode != -1 && mode != -2)
		env->ThrowError("TDeint:  mode must be set to -2, -1, 0, or 1!");
	if (order != 0 && order != 1 && order != -1)
		env->ThrowError("TDeint:  order must be set to 0, 1, or -1!");
	if (field != 0 && field != 1 && field != -1)
		env->ThrowError("TDeint:  field must be set to 0, 1, or -1!");
	if (map < 0 || map > 2)
		env->ThrowError("TDeint:  map option must be set to 0, 1, or 2!");
	if (ovrDefault != 0 && ovrDefault != 1)
		env->ThrowError("TDeint:  ovrDefault must be set to either 0 or 1!");
	if (type != 0 && type != 1 && type != 2 && type != 3)
		env->ThrowError("TDeint:  type must be set to either 0, 1, 2, or 3!");
	if (mtnmode < 0 || mtnmode > 3)
		env->ThrowError("TDeint:  mtnmode must be set to either 0, 1, 2, or 3!");
	if (vi.height&1 || vi.width&1)
		env->ThrowError("TDeint:  width and height must be multiples of 2!");
	if (link < 0 || link > 3)
		env->ThrowError("TDeint:  link must be set to 0, 1, 2, or 3!");
	if (blockx != 4 && blockx != 8 && blockx != 16 && blockx != 32 && blockx != 64 && 
		blockx != 128 && blockx != 256 && blockx != 512 && blockx != 1024 && blockx != 2048)
		env->ThrowError("TDeint:  illegal blockx size!");
	if (blocky != 4 && blocky != 8 && blocky != 16 && blocky != 32 && blocky != 64 && 
		blocky != 128 && blocky != 256 && blocky != 512 && blocky != 1024 && blocky != 2048)
		env->ThrowError("TDeint:  illegal blocky size!");
	if (APType < 0 || APType > 2)
		env->ThrowError("TDeint:  APType must be set to 0, 1, or 2!");
	child->SetCacheHints(CACHE_RANGE, 4);
	//useClip2 = false;
	if ((hints || !full) && mode == 0 && clip2)
	{
		const VideoInfo& vi1 = clip2->GetVideoInfo();
		if (vi1.height != vi.height || vi1.width != vi.width)
			env->ThrowError("TDeint:  width and height of clip2 must equal that of the input clip!");
		if (!vi1.IsYV12() && !vi1.IsYUY2())
			env->ThrowError("TDeint:  YV12 and YUY2 data only (clip2)!");
		if ((vi.IsYV12() && vi1.IsYUY2()) || (vi.IsYUY2() && vi1.IsYV12()))
			env->ThrowError("TDeint:  colorspace of clip2 doesn't match that of the input clip!");
		if (vi.num_frames != vi1.num_frames)
			env->ThrowError("TDeint:  number of frames in clip2 doesn't match that of the input clip!");
		useClip2 = true;
	}
	xhalf = blockx >> 1;
	yhalf = blocky >> 1;
	xshift = blockx == 4 ? 2 : blockx == 8 ? 3 : blockx == 16 ? 4 : blockx == 32 ? 5 :
		blockx == 64 ? 6 : blockx == 128 ? 7 : blockx == 256 ? 8 : blockx == 512 ? 9 : 
		blockx == 1024 ? 10 : 11;
	yshift = blocky == 4 ? 2 : blocky == 8 ? 3 : blocky == 16 ? 4 : blocky == 32 ? 5 :
		blocky == 64 ? 6 : blocky == 128 ? 7 : blocky == 256 ? 8 : blocky == 512 ? 9 : 
		blocky == 1024 ? 10 : 11;
	if (((!full && mode == 0) || tryWeave) && mode >= 0)
	{
		cArray = (int *)_aligned_malloc((((vi.width+xhalf)>>xshift)+1)*(((vi.height+yhalf)>>yshift)+1)*4*sizeof(int), 32);
		if (cArray == NULL) env->ThrowError("TDeint:  malloc failure!");
	}
	if (vi.IsYUY2())
	{
		xhalf *= 2;
		++xshift;
	}
	vi.SetFieldBased(false);
	nfrms = nfrms2 = vi.num_frames - 1;
	accumP = accumN = 0;
	cthresh6 = cthresh * 6;
	passHint = 0xFFFFFFFF;
	autoFO = false;
	if (mode < 0) 
	{
		vi.height *= 2;
		field = 1;
	}
	if (order == -1) autoFO = true;
	if (mode == 1)
	{
		vi.num_frames *= 2;
		nfrms2 = vi.num_frames - 1;
		vi.SetFPS(vi.fps_numerator*2, vi.fps_denominator);
	}
	else if (field == -1)
	{
		// telecide matches off the bottom field so we want field=0 in that case.
		// tfm can match off top or bottom, but it will indicate which in its hints
		// and field is adjusted appropriately then... so we use field=0 by default
		// if hints=true.  Otherwise, if hints=false, we default to field = 1.
		if (hints) field = 0;
		else field = 1;
	}
	orderS = order; 
	PRM(fieldS) = PRM(field); 
	mthreshLS = mthreshL; 
	mthreshCS = mthreshC;
	typeS = type;
	if (debug)
	{
		sprintf(buf,"TDeint:  %s (%s) by tritical\n", VERSION, DATE);
		OutputDebugString(buf);
		sprintf(buf,"TDeint:  mode = %d (%s)\n", mode, mode == 0 ? "normal - same rate" : 
				mode == 1 ? "bob - double rate" : mode == -2 ? "upsize - ELA" : "upsize - ELA-2");
		OutputDebugString(buf);
	}
	
noovrexit:
	_asm emms;
}
#endif

        //EOF
