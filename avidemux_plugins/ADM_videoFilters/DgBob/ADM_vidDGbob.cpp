/*
	DGBob() plugin for Avisynth -- Smart bob filter. This filter splits
	each field of the source into its own frame and then adaptively
	creates the missing lines either by interpolating the current field
	or by using the previous field's data. The filter attempts with some
	success to mitigate the flutter that bobbing produces.

	Copyright (C) 2003 Donald A. Graft

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "DIA_factory.h"
typedef struct DGBobparam
{
        uint32_t  thresh;// low=more flickering, less jaggie
        uint32_t  order; //0 : Bottom field first, 1 top field first        
        uint32_t  mode;  // 0 keep # of frames, 1 *2 fps & *2 frame, 2  #*2, fps*150% slow motion
        uint32_t  ap;    // Extra artifact check, better not to use
}DGBobparam;

class DGbob : public AVDMGenericVideoStream
{
       
        DGBobparam      *_param;        
        
        VideoCache      *vidCache;
       
        void            update(void); 
public:
                                
                        DGbob(AVDMGenericVideoStream *in,CONFcouple *couples);    
                        ~DGbob(void);
        uint8_t         getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                ADMImage *data,uint32_t *flags);
        
        char            *printConf( void );
        uint8_t         configure(AVDMGenericVideoStream *in);
        uint8_t         getCoupledConf( CONFcouple **couples);
};


//********** Register chunk ************
static FILTER_PARAM dgbobParam={4,{"order","mode","thresh","ap"}};


VF_DEFINE_FILTER(DGbob,dgbobParam,
				dgbob,
				QT_TR_NOOP("DG Bob"),
				1,
				VF_INTERLACING,
				QT_TR_NOOP("Donald Graft Bob."));
//************************************

/*************************************/
uint8_t DGbob::configure(AVDMGenericVideoStream *in)
{
        _in=in;
#define PX(x) &(_param->x)
  
     diaMenuEntry menuField[2]={{0,QT_TR_NOOP("Top"),NULL},
                             {1,QT_TR_NOOP("Bottom"),NULL}
                          };
  
     diaMenuEntry menuMode[3]={{0,QT_TR_NOOP("Keep nb of frames and fps"),NULL},
                            {1,QT_TR_NOOP("Double nb of frames and fps"),NULL},
                            {2,QT_TR_NOOP("Double nb of frames (slow motion)"),NULL}
                          };
                          
    diaElemMenu     menu1(PX(order),QT_TR_NOOP("_Field order:"), 2,menuField);
    diaElemMenu     menu2(PX(mode),QT_TR_NOOP("_Mode:"), 3,menuMode);
    diaElemUInteger threshold(PX(thresh),QT_TR_NOOP("_Threshold:"),0,255);
    diaElemToggle  extra(PX(ap),QT_TR_NOOP("_Extra"),QT_TR_NOOP("Extra check, avoid using it"));
    
      diaElem *elems[4]={&menu1,&menu2,&threshold ,&extra};
   if(diaFactoryRun(QT_TR_NOOP("DGBob"),4,elems))
  {
                update();
                return 1;
        }
        return 0;
        
}

char *DGbob::printConf( void )
{
      ADM_FILTER_DECLARE_CONF(" DGBob mode:%d order:%d thresh:%d\n",_param->mode,_param->order,_param->thresh);
        
}


DGbob::DGbob(AVDMGenericVideoStream *in,CONFcouple *couples)              

{
                
                int count = 0;
                char buf[80];
                unsigned int *p;

                _in=in;         
                memcpy(&_info,_in->getInfo(),sizeof(_info));    
                _info.encoding=1;
                _uncompressed=NULL;             
                _info.encoding=1;
                _info.fps1000*=2;
                _info.nb_frames*=2;
                
                                
                vidCache=new VideoCache(7,in);
                _param= new DGBobparam;
                if(couples)
                {
                        GET(order);
                        GET(mode);
                        GET(thresh);
                        GET(ap);
                }
                else
                {
                        _param->order=1;
                        _param->mode=0;
                        _param->thresh=12;
                        _param->ap=0;
                }
                update();
}
void DGbob::update(void)
{
                memcpy(&_info,_in->getInfo(),sizeof(_info));    
                _info.encoding=1;
                switch(_param->mode)
                {
                        case 0:
                                break;
                        case 1:
                                _info.fps1000*=2;
                                _info.nb_frames*=2;
                                break;
                        case 2:
                                _info.nb_frames*=2;
                                break;
                        default: ADM_assert(0);

                }
             
}
//________________________________________________________
uint8_t DGbob::getCoupledConf( CONFcouple **couples)
{
        *couples=NULL;
        *couples=new CONFcouple(4);
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
        
        CSET(order);
        CSET(mode);
        CSET(thresh);
        CSET(ap);

        return 1;
}
//________________________________________________________
DGbob::~DGbob(void)
{
                
                if(vidCache) delete vidCache;                
                vidCache=NULL;   
                if(_param) delete _param;
                _param=NULL;                             
}
uint8_t DGbob::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                ADMImage *data,uint32_t *flags)
{
        ADMImage *src,*prv,*prvprv,*nxt,*nxtnxt,*dst;
	uint32_t n,num_frames;

        if(frame>=_info.nb_frames) return 0;
        
        num_frames=_in->getInfo()->nb_frames;   // ??

	if (_param->mode == 0) n = frame;
	else n = frame/2;

        src=vidCache->getImage(n);
        prv=vidCache->getImage(n > 0 ? n - 1 :0 );
        prvprv=vidCache->getImage(n > 1 ? n - 2 : 0);
        nxt=vidCache->getImage(n < num_frames - 1 ? n + 1 : num_frames - 1);
        nxtnxt=vidCache->getImage(n < num_frames - 2 ? n + 2 : num_frames - 1);


/*
	PVideoFrame src = child->GetFrame(n, env);
	PVideoFrame prv = child->GetFrame(n > 0 ? n - 1 : 0, env);
	PVideoFrame prvprv = child->GetFrame(n > 1 ? n - 2 : 0, env);
	PVideoFrame nxt = child->GetFrame(n < vi.num_frames - 1 ? n + 1 : vi.num_frames - 1, env);
	PVideoFrame nxtnxt = child->GetFrame(n < vi.num_frames - 2 ? n + 2 : vi.num_frames - 1, env);
    PVideoFrame dst = env->NewVideoFrame(vi);
*/
    const unsigned char *srcp, *srcp_saved, *srcpp, *srcpn;
	const unsigned char *prvp, *prvpp, *prvpn, *prvprvp, *prvprvpp, *prvprvpn;
	const unsigned char *nxtp, *nxtpp, *nxtpn, *nxtnxtp, *nxtnxtpp, *nxtnxtpn;
    unsigned char *dstp, *dstp_saved;
 
    int src_pitch, dst_pitch, w, h;
	int x, y, z, v1, v2, D = _param->thresh, T = 6, AP = 30;
	int plane;

        uint32_t ww,hh;
	// Try making D a function of the average value of the comparands in
	// order to make the margin larger in darker areas, where we can't see as
	// much combing.

	// Force deinterlacing of the first and last frames.
	if (n == 0 || n == num_frames - 1) D = 0;
        dst=data;
	for (z = 0; z < 3; z++)
	{
/*
		if (z == 0) plane = PLANAR_Y;
		else if (z == 1) plane = PLANAR_U;
		else plane = PLANAR_V;

*/
                switch(z)
                {
                        case 0:
                                ww=_info.width;
                                hh=_info.height;
                                srcp_saved = srcp = YPLANE(src);                                
                                dstp_saved = dstp = YPLANE(dst);
                                src_pitch = ww;
                                dst_pitch = ww;
                                w = ww;
                                h = hh;
                                break;
                        case 1:
                        case 2:
                                ww=_info.width>>1;
                                hh=_info.height>>1;
                                if(z==1)
                                {
                                        srcp_saved = srcp = UPLANE(src);
                                        dstp_saved = dstp = UPLANE(dst);
                                }
                                else
                                {
                                        srcp_saved = srcp = VPLANE(src);
                                        dstp_saved = dstp = VPLANE(dst);
                                }
                                src_pitch = ww;
                                dst_pitch = ww;
                                w = ww;
                                h = hh;
                                break;
                }
/*
		srcp_saved = srcp = src->GetReadPtr(plane);
		src_pitch = src->GetPitch(plane);
		dstp_saved = dstp = dst->GetWritePtr(plane);
		dst_pitch = dst->GetPitch(plane);
		w = dst->GetRowSize(plane);
		h = dst->GetHeight(plane);
*/
		if ((_param->mode > 0) && (frame & 1))
		{
			// Process odd-numbered frames.
			// Copy field from current frame.
			srcp = srcp_saved +_param->order * src_pitch;
			dstp = dstp_saved +_param->order * dst_pitch;
			for (y = 0; y < h; y+=2)
			{
				memcpy(dstp, srcp, w);
				srcp += 2*src_pitch;
				dstp += 2*dst_pitch;
			}
			// Copy through the line that will be missed below.
			memcpy(dstp_saved + (1-_param->order)*(h-1)*dst_pitch, srcp_saved + (1-_param->order)*(h-1)*src_pitch, w);
			/* For the other field choose adaptively between using the previous field
			   or the interpolant from the current field. */
                        
			//prvp = prv->GetReadPtr(plane) + src_pitch + order*src_pitch;
                        switch(z)
                        {
                                case 0:prvp = YPLANE(prv) + src_pitch + _param->order*src_pitch;break;
                                case 1:prvp = UPLANE(prv) + src_pitch + _param->order*src_pitch;break;
                                case 2:prvp = VPLANE(prv) + src_pitch + _param->order*src_pitch;break;
                        }
			prvpp = prvp - src_pitch;
			prvpn = prvp + src_pitch;
			//prvprvp = prvprv->GetReadPtr(plane) + src_pitch + order*src_pitch;
                         switch(z)
                        {
                                case 0:prvprvp = YPLANE(prvprv) + src_pitch + _param->order*src_pitch;break;
                                case 1:prvprvp = UPLANE(prvprv) + src_pitch + _param->order*src_pitch;break;
                                case 2:prvprvp = VPLANE(prvprv) + src_pitch + _param->order*src_pitch;break;
                        }

			prvprvpp = prvprvp - src_pitch;
			prvprvpn = prvprvp + src_pitch;

			//nxtp = nxt->GetReadPtr(plane) + src_pitch + order*src_pitch;
                        switch(z)
                        {
                                case 0:nxtp = YPLANE(nxt) + src_pitch + _param->order*src_pitch;break;
                                case 1:nxtp = UPLANE(nxt) + src_pitch + _param->order*src_pitch;break;
                                case 2:nxtp = VPLANE(nxt) + src_pitch + _param->order*src_pitch;break;
                        }

			nxtpp = nxtp - src_pitch;
			nxtpn = nxtp + src_pitch;
			//nxtnxtp = nxtnxt->GetReadPtr(plane) + src_pitch + order*src_pitch;
                        switch(z)
                        {
                                case 0:nxtnxtp = YPLANE(nxtnxt) + src_pitch + _param->order*src_pitch;break;
                                case 1:nxtnxtp = UPLANE(nxtnxt) + src_pitch + _param->order*src_pitch;break;
                                case 2:nxtnxtp = VPLANE(nxtnxt) + src_pitch + _param->order*src_pitch;break;
                        }
			nxtnxtpp = nxtnxtp - src_pitch;
			nxtnxtpn = nxtnxtp + src_pitch;
			srcp =  srcp_saved + src_pitch + _param->order*src_pitch;
			srcpp = srcp - src_pitch;
			srcpn = srcp + src_pitch;
			dstp =  dstp_saved + dst_pitch + _param->order*dst_pitch;
			for (y = 0; y < h - 2; y+=2)
			{
				for (x = 0; x < w; x++)
				{
					if (
						abs(srcp[x] - nxtp[x]) < D
//						&& abs(srcp[x] - nxtnxtp[x]) < D
//						&& abs(prvp[x] - nxtp[x]) < D
						&& abs(srcpn[x] - prvprvpn[x]) < D
						&& abs(srcpp[x] - prvprvpp[x]) < D
						&& abs(srcpn[x] - nxtnxtpn[x]) < D
						&& abs(srcpp[x] - nxtnxtpp[x]) < D
						&& abs(srcpn[x] - prvpn[x]) < D
						&& abs(srcpp[x] - prvpp[x]) < D
						&& abs(srcpn[x] - nxtpn[x]) < D
						&& abs(srcpp[x] - nxtpp[x]) < D
					   )
					{
						if (_param->ap == true)
						{
							v1 = (int) srcp[x] - AP;
							if (v1 < 0) v1 = 0; 
							v2 = (int) srcp[x] + AP;
							if (v2 > 235) v2 = 235; 
							if ((v1 > srcpp[x] && v1 > srcpn[x]) || (v2 < srcpp[x] && v2 < srcpn[x]))
							{
								dstp[x] = ((int)srcpp[x] + srcpn[x]) >> 1;
//								if (x & 1) dstp[x] = 100; else dstp[x] = 235;
							}
							else
							{
								dstp[x] = srcp[x];
//								if (x & 1) dstp[x] = 100; else dstp[x] = 235;
							}
						}
						else
						{
							dstp[x] = srcp[x];
//							if (x & 1) dstp[x] = 100; else dstp[x] = 235;
						}
					}
					else
					{
						v1 = (int) srcp[x] - T;
						if (v1 < 0) v1 = 0; 
						v2 = (int) srcp[x] + T;
						if (v2 > 235) v2 = 235; 
						if ((v1 > srcpp[x] && v1 > srcpn[x]) || (v2 < srcpp[x] && v2 < srcpn[x]))
						{
							dstp[x] = ((int)srcpp[x] + srcpn[x]) >> 1;
						}
						else
						{
							dstp[x] = srcp[x];
//							if (x & 1) dstp[x] = 128; else dstp[x] = 235;
						}
					}
				}
				prvp    += 2*src_pitch;
				prvpp    += 2*src_pitch;
				prvpn    += 2*src_pitch;
				prvprvpp    += 2*src_pitch;
				prvprvpn    += 2*src_pitch;
				nxtp    += 2*src_pitch;
				nxtpp    += 2*src_pitch;
				nxtpn    += 2*src_pitch;
				nxtnxtpp    += 2*src_pitch;
				nxtnxtpn    += 2*src_pitch;
				srcp    += 2*src_pitch;
				srcpp   += 2*src_pitch;
				srcpn   += 2*src_pitch;
				dstp    += 2*dst_pitch;
			}
		}
		else
		{
			// Process even-numbered frames.
			// Copy field from current frame.
			srcp = srcp_saved + (1-_param->order) * src_pitch;
			dstp = dstp_saved + (1-_param->order) * dst_pitch;
			for (y = 0; y < h; y+=2)
			{
				memcpy(dstp, srcp, w);
				srcp += 2*src_pitch;
				dstp += 2*dst_pitch;
			}
			// Copy through the line that will be missed below.
			memcpy(dstp_saved + _param->order*(h-1)*dst_pitch, srcp_saved + _param->order*(h-1)*src_pitch, w);
			/* For the other field choose adaptively between using the previous field
			   or the interpolant from the current field. */
			//prvp = prv->GetReadPtr(plane) + src_pitch + (1-order)*src_pitch;
                        switch(z)
                        {
                                case 0:prvp = YPLANE(prv) + src_pitch + (1-_param->order)*src_pitch;break;
                                case 1:prvp = UPLANE(prv) + src_pitch + (1-_param->order)*src_pitch;break;
                                case 2:prvp = VPLANE(prv) + src_pitch + (1-_param->order)*src_pitch;break;
                        }
			prvpp = prvp - src_pitch;
			prvpn = prvp + src_pitch;
			// prvprvp = prvprv->GetReadPtr(plane) + src_pitch + (1-order)*src_pitch;
                        switch(z)
                        {
                                case 0:prvprvp = YPLANE(prvprv) + src_pitch + (1-_param->order)*src_pitch;break;
                                case 1:prvprvp = UPLANE(prvprv) + src_pitch + (1-_param->order)*src_pitch;break;
                                case 2:prvprvp = VPLANE(prvprv) + src_pitch + (1-_param->order)*src_pitch;break;
                        }
                        
			prvprvpp = prvprvp - src_pitch;
			prvprvpn = prvprvp + src_pitch;
			//nxtp = nxt->GetReadPtr(plane) + src_pitch + (1-order)*src_pitch;
                        switch(z)
                        {
                                case 0:nxtp = YPLANE(nxt) + src_pitch + (1-_param->order)*src_pitch;break;
                                case 1:nxtp = UPLANE(nxt) + src_pitch + (1-_param->order)*src_pitch;break;
                                case 2:nxtp = VPLANE(nxt) + src_pitch + (1-_param->order)*src_pitch;break;
                        }
			nxtpp = nxtp - src_pitch;
			nxtpn = nxtp + src_pitch;
			//nxtnxtp = nxtnxt->GetReadPtr(plane) + src_pitch + (1-order)*src_pitch;
                         switch(z)
                        {
                                case 0:nxtnxtp = YPLANE(nxtnxt) + src_pitch + (1-_param->order)*src_pitch;break;
                                case 1:nxtnxtp = UPLANE(nxtnxt) + src_pitch + (1-_param->order)*src_pitch;break;
                                case 2:nxtnxtp = VPLANE(nxtnxt) + src_pitch + (1-_param->order)*src_pitch;break;
                        }
			nxtnxtpp = nxtnxtp - src_pitch;
			nxtnxtpn = nxtnxtp + src_pitch;
			srcp =  srcp_saved + src_pitch + (1-_param->order)*src_pitch;
			srcpp = srcp - src_pitch;
			srcpn = srcp + src_pitch;
			dstp =  dstp_saved + dst_pitch + (1-_param->order)*dst_pitch;
			for (y = 0; y < h - 2; y+=2)
			{
				for (x = 0; x < w; x++)
				{
					if (
						abs(srcp[x] - prvp[x]) < D
//						&& abs(srcp[x] - prvprvp[x]) < D
//						&& abs(prvp[x] - nxtp[x]) < D
						&& abs(srcpn[x] - prvprvpn[x]) < D
						&& abs(srcpp[x] - prvprvpp[x]) < D
						&& abs(srcpn[x] - nxtnxtpn[x]) < D
						&& abs(srcpp[x] - nxtnxtpp[x]) < D
						&& abs(srcpn[x] - prvpn[x]) < D
						&& abs(srcpp[x] - prvpp[x]) < D
						&& abs(srcpn[x] - nxtpn[x]) < D
						&& abs(srcpp[x] - nxtpp[x]) < D
					   )
					{
						if (_param->ap == true)
						{
							v1 = (int) prvp[x] - AP;
							if (v1 < 0) v1 = 0; 
							v2 = (int) prvp[x] + AP;
							if (v2 > 235) v2 = 235; 
							if ((v1 > srcpp[x] && v1 > srcpn[x]) ||	(v2 < srcpp[x] && v2 < srcpn[x]))
							{
								dstp[x] = ((int)srcpp[x] + srcpn[x]) >> 1;
//								if (x & 1) dstp[x] = 100; else dstp[x] = 235;
							}
							else
							{
								dstp[x] = prvp[x];
//								if (x & 1) dstp[x] = 128; else dstp[x] = 235;
							}
						}
						else
						{
							dstp[x] = prvp[x];
//							if (x & 1) dstp[x] = 128; else dstp[x] = 235;
						}
					}
					else
					{
						v1 = (int) prvp[x] - T;
						if (v1 < 0) v1 = 0; 
						v2 = (int) prvp[x] + T;
						if (v2 > 235) v2 = 235; 
						if ((v1 > srcpp[x] && v1 > srcpn[x]) ||	(v2 < srcpp[x] && v2 < srcpn[x]))
						{
							dstp[x] = ((int)srcpp[x] + srcpn[x]) >> 1;
						}
						else
						{
							dstp[x] = prvp[x];
//							if (x & 1) pp[x] = 128; else dstp[x] = 235;
						}
					}
				}
				prvp    += 2*src_pitch;
				prvpp    += 2*src_pitch;
				prvpn    += 2*src_pitch;
				prvprvpp    += 2*src_pitch;
				prvprvpn    += 2*src_pitch;
				nxtp    += 2*src_pitch;
				nxtpp    += 2*src_pitch;
				nxtpn    += 2*src_pitch;
				nxtnxtpp    += 2*src_pitch;
				nxtnxtpn    += 2*src_pitch;
				srcp    += 2*src_pitch;
				srcpp   += 2*src_pitch;
				srcpn   += 2*src_pitch;
				dstp    += 2*dst_pitch;
			}
		}
	}
        vidCache->unlockAll();
	return 1;
}
//EOF



