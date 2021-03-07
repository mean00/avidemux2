/**/
/***************************************************************************
                          ADM_vidASharp  -  description
                             -------------------
    Port of avisynth one

 ***************************************************************************/
/*

        Copyright (C) 2003 Donald A. Graft
        Copyright (C) 2002 Marc Fauconneau
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

#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "asharp.h"
#include "asharp_desc.cpp"
#define uc uint8_t
/**
    \class ASharp
*/
class ASharp : public ADM_coreVideoFilter
{
private:


        asharp          _param;
        int32_t         T,D,B,B2;
        uint8_t         *lineptr;

public:
                            ASharp(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
                            ~ASharp();

               void         update(void);
       virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
       virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual void         setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;                 /// Start graphical user interface

        static void         reset(asharp *cfg);
        static void         asharp_run_c(uc* planeptr, int pitch, int height, int width,
                                        int T,int D, int B, int B2, bool bf, uint8_t *lineptr);
};



// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ASharp,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_SHARPNESS,            // Category
                        "asharp",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("asharp","Asharp"),            // Display name
                        QT_TRANSLATE_NOOP("asharp","Adaptative sharpener by MarcFD.") // Description
                    );

/**
    \fn ASharp
    \brief ctor
*/
ASharp::ASharp(ADM_coreVideoFilter *in,CONFcouple *couples) : ADM_coreVideoFilter(in,couples)
{
        if(!couples || !ADM_paramLoad(couples,asharp_param,&_param))
            reset(&_param);
        lineptr=new uint8_t[info.width];
        update();
        ADM_info("%s\n",getConfiguration());
}
/**
    \fn dtor
    \brief recompute parameters
*/

ASharp::~ASharp(void)
{
    delete [] lineptr;
}
/**
    \fn reset
*/
void ASharp::reset(asharp *cfg)
{
    cfg->t = 2;
    cfg->d = 4;
    cfg->b = -1;
    cfg->bf = false;
    cfg->d_enabled = true;
    cfg->b_enabled = false;
}
/**
    \fn update
    \brief recompute parameters
*/

void ASharp::update( void)
{
                if(_param.t<0) _param.t=0;
                if(_param.t>32.) _param.t=32.;
                if(_param.d<0) _param.d=0;
                if(_param.d>16.) _param.d=16.;
                if(_param.b>4.) _param.b=4;

#define ALMOST_ZERO 0.002
                // fake a non-zero value for param.d
                float faked = _param.d;
                if(faked < ALMOST_ZERO) faked = ALMOST_ZERO;

                // parameters floating point to fixed point conversion
                T = (int)(_param.t*(4<<7));
                D = _param.d_enabled ? (int)(faked*(4<<7)) : 0;
                B = _param.b_enabled ? (int)(256-_param.b*64) : 256;
                B2= _param.b_enabled ? (int)(256-_param.b*48) : 256;

                // clipping (recommended for SIMD code)

                if (T<-(4<<7)) T = -(4<<7); // yes, negatives values are accepted
                if (D<0) D = 0;
                if (B<0) B = 0;
                if (B2<0) B2 = 0;

                if (T>(32*(4<<7))) T = (32*(4<<7));
                if (D>(16*(4<<7))) D = (16*(4<<7));
                if (B>256) B = 256;
                if (B2>256) B2 = 256;

}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         ASharp::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, asharp_param,&_param);
}

void ASharp::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, asharp_param, &_param);
}

extern uint8_t DIA_getASharp(asharp *param, ADM_coreVideoFilter *in);

/**
    \fn configure
*/
bool ASharp::configure(void)
{
    if(DIA_getASharp(&_param, previousFilter))
    {
        update();
        ADM_info("ASharp %s\n",getConfiguration());
        return true;
    }
    return false;
}
/**
    \fn getConfiguration
*/
const char   *ASharp::getConfiguration(void)
{
#define CONF_MAX_LEN 256
    static char s[CONF_MAX_LEN];
    int len = CONF_MAX_LEN;
    char *c = s;

    s[len-1]=0;
    snprintf(c,len,"Threshold: %.02f ",_param.t);
    len-=strlen(c);
    if(len<2) return s;

    c+=strlen(c);
    if(_param.d_enabled)
        snprintf(c,len," Adaptive strength: %.02f ",_param.d);
    else
        snprintf(c,len," Adaptive strength: disabled ");
    len-=strlen(c);
    if(len<2) return s;

    c+=strlen(c);
    if(_param.b_enabled)
        snprintf(c,len," Block adaptive: %.02f ",_param.b);
    else
        snprintf(c,len," Block adaptive: disabled ");
    len-=strlen(c);
    if(len<2) return s;

    c+=strlen(c);
    snprintf(c,len," HQBF: %s",_param.bf? "enabled" : "disabled");
    return s;

}
/**
    \fn getNextFrame
*/
 bool         ASharp::getNextFrame(uint32_t *fn,ADMImage *image)
{
ADMImage *dst;

        dst=image;

        if(!previousFilter->getNextFrame(fn,dst)) return false;

        asharp_run_c(   dst->GetWritePtr(PLANAR_Y),
                        dst->GetPitch(PLANAR_Y),
                        info.height,
                        info.width,
                        T,
                        D,
                        B,
                        B2,
                        _param.bf,lineptr);
        return 1;
}

/**
    \fn asharp_run_c
*/
void ASharp::asharp_run_c(uc* planeptr, int pitch,
                        int height,     int width,
                        int     T,      int D, int B, int B2, bool bf , uint8_t *lineptr)
{
    int p = pitch;
    int y,x;
    uc* cfp = planeptr+pitch;
    uc* lp = lineptr;
    // initialize buffer
    memcpy(lp,planeptr,width);
    for (y=1;y<height-2;y++) 
    {
        int last = cfp[0];
        for (x=1;x<width-2;x++) 
        {

            int avg = 0;
            int dev = 0;
            int T2;
            int diff;
            int tmp;

            avg += lp[x-1];
            avg += lp[x  ];
            avg += lp[x+1];
            avg += last;
            avg += cfp[x    ];
            avg += cfp[x  +1];
            avg += cfp[x+p-1];
            avg += cfp[x+p  ];
            avg += cfp[x+p+1];

            avg *= (65535/9);
            avg >>= 16;

            #define CHECK(A)  {int dif=abs(A-current);if(dif>dev) dev=dif;}
            uc current=cfp[x];
            int xmod8=x&7;
            int ymod8=y&7;
            if (bf) 
            {
                    if (ymod8) //if (y%8>0)
                    {
                            if (xmod8&7) CHECK(lp[x-1])
                            CHECK(lp[x  ])
                            if (xmod8<7) CHECK(lp[x+1])
                    }
                    if ((xmod8)) CHECK(last)
                    if (xmod8<7) CHECK(cfp[x  +1])
                    if (ymod8<7) 
                    {
                            if (xmod8>0) CHECK(cfp[x+p-1])
                            CHECK(cfp[x+p  ])
                            if (xmod8<7) CHECK(cfp[x+p+1])
                    }

            } else {
                    CHECK(cfp[x-p-1])
                    CHECK(cfp[x-p  ])
                    CHECK(cfp[x-p+1])
                    CHECK(last)
                    CHECK(cfp[x+1  ])
                    CHECK(cfp[x+p-1])
                    CHECK(cfp[x+p  ])
                    CHECK(cfp[x+p+1])
            }

            T2 = T;
            diff = current-avg;
            int D2 = D;


            switch(xmod8)
            {
                    case 6: D2=(D2*B2)>>8;break;
                    case 7: D2=(D2*B)>>8;break;
                    case 0: D2=(D2*B)>>8;break;
                    case 1: D2=(D2*B2)>>8;break;
                    default:break;
            }


            switch(ymod8)
            {
                    case 6: D2=(D2*B2)>>8;break;
                    case 7: D2=(D2*B)>>8;break;
                    case 0: D2=(D2*B)>>8;break;
                    case 1: D2=(D2*B2)>>8;break;
                    default:break;
            }

            int Da = -32+(D>>7);
            if (D>0) T2 = ((((dev<<7)*D2)>>16)+Da)<<4;

            if (T2>T) T2=T;
            if (T2<-32) T2=-32;

            tmp = (((diff<<7)*T2)>>16)+current;

            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            lp[x-1] = last;
            last = current;
            cfp[x] = tmp;
        }
        lp[x] = cfp[x];
        cfp += pitch;
    }
}
//**********************************
