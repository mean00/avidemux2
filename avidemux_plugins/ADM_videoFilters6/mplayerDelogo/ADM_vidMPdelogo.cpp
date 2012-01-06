/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "DIA_flyDialog.h"
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "delogo.h"
#include "delogo_desc.cpp"


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}

#include "ADM_vidMPdelogo.h"
/**
    \class ADMVideoMPdelogo
*/
extern bool DIA_getMpDelogo(delogo *param, ADM_coreVideoFilter *previousFilter);
class  MPDelogo:public ADM_coreVideoFilter
 {
 protected:
                    delogo          param;

 public:

                            MPDelogo(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
                            MPDelogo(ADM_coreVideoFilter *in,int x,int y)   ;
       virtual              ~MPDelogo();
       virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual bool         configure(void) ;                 /// Start graphical user interface     

 }     ;

extern uint8_t DIA_getMPdelogo(delogo *param,ADM_coreVideoFilter *in);
static void xdelogo(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int width, int height,
                   int logo_x, int logo_y, int logo_w, int logo_h, int band, int show, int direct) ;

//******************************************

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   MPDelogo,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_SHARPNESS,            // Category
                        "mpdelogo",            // internal name (must be uniq!)
                        "MPlayer delogo",            // Display name
                        QT_TR_NOOP("Blend a logo by interpolating its surrounding box.") // Description
                    );
//******************************************
/**
    \fn ASharp 
    \brief ctor
*/
MPDelogo::MPDelogo(ADM_coreVideoFilter *in,CONFcouple *couples) : ADM_coreVideoFilter(in,couples)
{
        if(!couples || !ADM_paramLoad(couples,delogo_param,&param))
		{
                param.xoff=0;
                param.yoff=0;
                param.lw = info.width>>1;
                param.lh = info.height>>1;
                param.band=4;
                param.show=0;
        }
}
/**
    \fn dtor 
    \brief recompute parameters
*/

MPDelogo::~MPDelogo(void)
{
        
}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         MPDelogo::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, delogo_param,&param);
}

/**
    \fn getConfiguration
*/
const char   *MPDelogo::getConfiguration(void)   
{
    static char s[256];
    snprintf(s,255,"x=%d y=%d w=%d h=%d",(int)param.xoff, (int)param.yoff,(int)param.lw,(int)param.lh);
    return s;        
}
/**
    \fn configure
*/
bool MPDelogo::configure(void)
{
uint8_t r=0;
        if( DIA_getMpDelogo(&param, previousFilter))
        {
                r=1;
        }
        return r;
}

/**
    \fn getNextFrame
*/
 bool         MPDelogo::getNextFrame(uint32_t *fn,ADMImage *data)
{
ADMImage *dst;

        dst=data;
        
        if(!previousFilter->getNextFrame(fn,dst)) return false;

        for(int i=0;i<3;i++)
        {
            ADM_PLANE p=(ADM_PLANE)i;
            int x=param.xoff,y=param.yoff;
            int w=param.lw,h=param.lh;
            int width=data->GetWidth(p);
            int height=data->GetHeight(p);
            int stride=data->GetPitch(p);
            if(i)
            {
                    x>>=1;y>>=1;w>>=1;h>>=1;
            }
//void xdelogo(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int width, int height,
//		   int logo_x, int logo_y, int logo_w, int logo_h, int band, int show, int direct)
            xdelogo(data->GetWritePtr(p), data->GetReadPtr(p),
                stride,stride,width,height,
                x,y,w,h, param.band, param.show, true);

        }
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

void xdelogo(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int width, int height,
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
