/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}

#include "ADM_vidMPdelogo.h"

class  ADMVideoMPdelogo:public AVDMGenericVideoStream
 {

 protected:
                                MPDELOGO_PARAM  *_param;
                                VideoCache      *vidCache;

 public:

                                ADMVideoMPdelogo(  AVDMGenericVideoStream *in,CONFcouple *setup);
                                ADMVideoMPdelogo(        AVDMGenericVideoStream *in,uint32_t x,uint32_t y);
                                virtual                 ~ADMVideoMPdelogo();
          virtual               uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                                ADMImage *data,uint32_t *flags);
                                uint8_t configure( AVDMGenericVideoStream *instream);
        virtual                 char    *printConf(void) ;

          virtual uint8_t       getCoupledConf( CONFcouple **couples);


 }     ;
extern uint8_t DIA_getMPdelogo(MPDELOGO_PARAM *param,AVDMGenericVideoStream *in);

static FILTER_PARAM mpdelogoParam={6,{"xoff","yoff","lw","lh","band","show"}};
//REGISTERX(VF_MISC, "mpdelogo",QT_TR_NOOP("MPlayer delogo"),
//QT_TR_NOOP("Blend a logo by interpolating its surrounding box."),VF_MPDELOGO,1,mpdelogo_create,mpdelogo_script);
//******************************************
VF_DEFINE_FILTER_UI(ADMVideoMPdelogo,mpdelogoParam,
                mpdelogo,
                QT_TR_NOOP("MPlayer delogo"),
                1,
                VF_MISC,
                QT_TR_NOOP("Blend a logo by interpolating its surrounding box."));

//******************************************


uint8_t ADMVideoMPdelogo::configure(AVDMGenericVideoStream * instream)
{
    UNUSED_ARG(instream);

    return DIA_getMPdelogo(_param,instream);
}

char *ADMVideoMPdelogo::printConf( void )
{
        ADM_FILTER_DECLARE_CONF(" MPlayer delogo : at (%d,%d) (%dx%d)",
                                _param->xoff,_param->yoff,_param->lw,_param->lh);
        
}
//_______________________________________________________________
ADMVideoMPdelogo::ADMVideoMPdelogo(
         AVDMGenericVideoStream *in,CONFcouple *couples)
{

        _in=in;
        memcpy(&_info,_in->getInfo(),sizeof(_info));
        _param=NEW(MPDELOGO_PARAM);
        vidCache=new VideoCache(4,_in);
        if(couples)
        {

                GET(xoff);
                GET(yoff);
                GET(lw);
                GET(lh);
                GET(show);
                GET(band);
        
        }
        else
        {
                _param->xoff=0;
                _param->yoff=0;
                _param->lw = _info.width>>1;
                _param->lh = _info.height>>1;
                _param->band=4;
                _param->show=0;
        }
        _info.encoding=1;
}


uint8_t ADMVideoMPdelogo::getCoupledConf( CONFcouple **couples)
{

                        ADM_assert(_param);
                        *couples=new CONFcouple(6);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
                CSET(xoff);
                CSET(yoff);
                CSET(lw);
                CSET(lh);
                CSET(show);
                CSET(band);
                return 1;

}
// ___ destructor_____________
ADMVideoMPdelogo::~ADMVideoMPdelogo()
{
        DELETE(_param);
        delete vidCache;
        vidCache=NULL;
}
static void delogo(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int width, int height,
                   int logo_x, int logo_y, int logo_w, int logo_h, int band, int show, int direct) ;

uint8_t ADMVideoMPdelogo::getFrameNumberNoAlloc(uint32_t frame,
                                uint32_t *len,
                                ADMImage *data,
                                uint32_t *flags)
{
                        if(frame>=_info.nb_frames) 
                        {
                                printf("MPdelogo : Filter : out of bound!\n");
                                return 0;
                        }
        
                        ADM_assert(_param);

ADMImage *curImage;
char txt[256];
                        curImage=vidCache->getImage(frame);
                        if(!curImage)
                        {
                                printf("MPdelogo : error getting frame\n");
                                return 0;
                        }
                delogo(YPLANE(data), YPLANE(curImage), _info.width, _info.width, _info.width, _info.height,
                        _param->xoff, _param->yoff, _param->lw, _param->lh, _param->band, _param->show,0);
                delogo(UPLANE(data), UPLANE(curImage), _info.width>>1, _info.width>>1, _info.width>>1,
                 _info.height>>1,_param->xoff>>1, _param->yoff>>1, _param->lw>>1,
                 _param->lh>>1, _param->band>>1, _param->show,0);
                delogo(VPLANE(data), VPLANE(curImage), _info.width>>1, _info.width>>1, _info.width>>1,
                 _info.height>>1,_param->xoff>>1, _param->yoff>>1, _param->lw>>1,
                 _param->lh>>1, _param->band>>1, _param->show,0);
                
                        vidCache->unlockAll();
        return 1;
}


/*
  Copyright (C) 2002 Jindrich Makovicka <makovick@kmlinux.fjfi.cvut.cz>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* A very simple tv station logo remover */



//===========================================================================//


#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

void delogo(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int width, int height,
		   int logo_x, int logo_y, int logo_w, int logo_h, int band, int show, int direct) {
    int y, x;
    int interp, dist;
    uint8_t *xdst, *xsrc;
    
    uint8_t *topleft, *botleft, *topright;
    int xclipl, xclipr, yclipt, yclipb;
    int logo_x1, logo_x2, logo_y1, logo_y2;
    
    xclipl = MAX(-logo_x, 0);
    xclipr = MAX(logo_x+logo_w-width, 0);
    yclipt = MAX(-logo_y, 0);
    yclipb = MAX(logo_y+logo_h-height, 0);

    logo_x1 = logo_x + xclipl;
    logo_x2 = logo_x + logo_w - xclipr;
    logo_y1 = logo_y + yclipt;
    logo_y2 = logo_y + logo_h - yclipb;

    topleft = src+logo_y1*srcStride+logo_x1;
    topright = src+logo_y1*srcStride+logo_x2-1;
    botleft = src+(logo_y2-1)*srcStride+logo_x1;

    if (!direct) memcpy(dst, src,(width*height*3)/2); // width, height, dstStride, srcStride);

    dst += (logo_y1+1)*dstStride;
    src += (logo_y1+1)*srcStride;
    
    for(y = logo_y1+1; y < logo_y2-1; y++)
    {
	for (x = logo_x1+1, xdst = dst+logo_x1+1, xsrc = src+logo_x1+1; x < logo_x2-1; x++, xdst++, xsrc++) {
	    interp = ((topleft[srcStride*(y-logo_y-yclipt)]
		       + topleft[srcStride*(y-logo_y-1-yclipt)]
		       + topleft[srcStride*(y-logo_y+1-yclipt)])*(logo_w-(x-logo_x))/logo_w
		      + (topright[srcStride*(y-logo_y-yclipt)]
			 + topright[srcStride*(y-logo_y-1-yclipt)]
			 + topright[srcStride*(y-logo_y+1-yclipt)])*(x-logo_x)/logo_w
		      + (topleft[x-logo_x-xclipl]
			 + topleft[x-logo_x-1-xclipl]
			 + topleft[x-logo_x+1-xclipl])*(logo_h-(y-logo_y))/logo_h
		      + (botleft[x-logo_x-xclipl]
			 + botleft[x-logo_x-1-xclipl]
			 + botleft[x-logo_x+1-xclipl])*(y-logo_y)/logo_h
		)/6;
/*		interp = (topleft[srcStride*(y-logo_y)]*(logo_w-(x-logo_x))/logo_w
			  + topright[srcStride*(y-logo_y)]*(x-logo_x)/logo_w
			  + topleft[x-logo_x]*(logo_h-(y-logo_y))/logo_h
			  + botleft[x-logo_x]*(y-logo_y)/logo_h
			  )/2;*/
	    if (y >= logo_y+band && y < logo_y+logo_h-band && x >= logo_x+band && x < logo_x+logo_w-band) {
		    *xdst = interp;
	    } else {
		dist = 0;
		if (x < logo_x+band) dist = MAX(dist, logo_x-x+band);
		else if (x >= logo_x+logo_w-band) dist = MAX(dist, x-(logo_x+logo_w-1-band));
		if (y < logo_y+band) dist = MAX(dist, logo_y-y+band);
		else if (y >= logo_y+logo_h-band) dist = MAX(dist, y-(logo_y+logo_h-1-band));
		*xdst = (*xsrc*dist + interp*(band-dist))/band;
		if (show && (dist == band-1)) *xdst = 0;
	    }
	}

	dst+= dstStride;
	src+= srcStride;
    }
}

#if 0
static int put_image(struct vf_instance_s* vf, mp_image_t *mpi){
    mp_image_t *dmpi;

    if(!(mpi->flags&MP_IMGFLAG_DIRECT)){
	// no DR, so get a new image! hope we'll get DR buffer:
	vf->dmpi=vf_get_image(vf->next,vf->priv->outfmt,
			      MP_IMGTYPE_TEMP, MP_IMGFLAG_ACCEPT_STRIDE,
			      mpi->w,mpi->h);
    }
    dmpi= vf->dmpi;

    delogo(dmpi->planes[0], mpi->planes[0], dmpi->stride[0], mpi->stride[0], mpi->w, mpi->h,
	   vf->priv->xoff, vf->priv->yoff, vf->priv->lw, vf->priv->lh, vf->priv->band, vf->priv->show,
	   mpi->flags&MP_IMGFLAG_DIRECT);
    delogo(dmpi->planes[1], mpi->planes[1], dmpi->stride[1], mpi->stride[1], mpi->w/2, mpi->h/2,
	   vf->priv->xoff/2, vf->priv->yoff/2, vf->priv->lw/2, vf->priv->lh/2, vf->priv->band/2, vf->priv->show,
	   mpi->flags&MP_IMGFLAG_DIRECT);
    delogo(dmpi->planes[2], mpi->planes[2], dmpi->stride[2], mpi->stride[2], mpi->w/2, mpi->h/2,
	   vf->priv->xoff/2, vf->priv->yoff/2, vf->priv->lw/2, vf->priv->lh/2, vf->priv->band/2, vf->priv->show,
	   mpi->flags&MP_IMGFLAG_DIRECT);

    vf_clone_mpi_attributes(dmpi, mpi);

    return vf_next_put_image(vf,dmpi);
}

static void uninit(struct vf_instance_s* vf){
    if(!vf->priv) return;

    free(vf->priv);
    vf->priv=NULL;
}

//===========================================================================//

static int query_format(struct vf_instance_s* vf, unsigned int fmt){
    switch(fmt)
    {
    case IMGFMT_YV12:
    case IMGFMT_I420:
    case IMGFMT_IYUV:
	return vf_next_query_format(vf,vf->priv->outfmt);
    }
    return 0;
}

static unsigned int fmt_list[]={
    IMGFMT_YV12,
    IMGFMT_I420,
    IMGFMT_IYUV,
    0
};

static int open(vf_instance_t *vf, char* args){
    int res;
    
    vf->config=config;
    vf->put_image=put_image;
    vf->get_image=get_image;
    vf->query_format=query_format;
    vf->uninit=uninit;
    if (!vf->priv)
    {
        vf->priv=malloc(sizeof(struct vf_priv_s));
	memset(vf->priv, 0, sizeof(struct vf_priv_s));
    }

    if (args) res = sscanf(args, "%d:%d:%d:%d:%d",
			   &vf->priv->xoff, &vf->priv->yoff,
			   &vf->priv->lw, &vf->priv->lh,
			   &vf->priv->band);
    if (args && (res != 5)) {
	uninit(vf);
	return 0; // bad syntax
    }

    mp_msg(MSGT_VFILTER, MSGL_V, "delogo: %d x %d, %d x %d, band = %d\n",
	   vf->priv->xoff, vf->priv->yoff,
	   vf->priv->lw, vf->priv->lh,
	   vf->priv->band);

    vf->priv->show = 0;

    if (vf->priv->band < 0) {
	vf->priv->band = 4;
	vf->priv->show = 1;
    }
    

    vf->priv->lw += vf->priv->band*2;
    vf->priv->lh += vf->priv->band*2;
    vf->priv->xoff -= vf->priv->band;
    vf->priv->yoff -= vf->priv->band;

    // check csp:
    vf->priv->outfmt=vf_match_csp(&vf->next,fmt_list,IMGFMT_YV12);
    if(!vf->priv->outfmt)
    {
	uninit(vf);
        return 0; // no csp match :(
    }

    return 1;
}

#define ST_OFF(f) M_ST_OFF(struct vf_priv_s,f)
static m_option_t vf_opts_fields[] = {
    { "x", ST_OFF(xoff), CONF_TYPE_INT, 0, 0, 0, NULL },
    { "y", ST_OFF(yoff), CONF_TYPE_INT, 0, 0, 0, NULL },
    { "w", ST_OFF(lw), CONF_TYPE_INT, 0, 0, 0, NULL },
    { "h", ST_OFF(lh), CONF_TYPE_INT, 0, 0, 0, NULL },
    { "t", ST_OFF(band), CONF_TYPE_INT, 0, 0, 0, NULL },
    { "band", ST_OFF(band), CONF_TYPE_INT, 0, 0, 0, NULL }, // alias
    { NULL, NULL, 0, 0, 0, 0, NULL }
};

static m_struct_t vf_opts = {
    "delogo",
    sizeof(struct vf_priv_s),
    &vf_priv_dflt,
    vf_opts_fields
};

vf_info_t vf_info_delogo = {
    "simple logo remover",
    "delogo",
    "Jindrich Makovicka, Alex Beregszaszi",
    "",
    open,
    &vf_opts
};
#endif
//===========================================================================//
