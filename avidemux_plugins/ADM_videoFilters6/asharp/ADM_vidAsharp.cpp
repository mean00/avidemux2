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

#include "DIA_flyDialog.h"
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
        
        ADMImage        *original;
        asharp          _param;
        int32_t         T,D,B,B2;
        
public:    
                        ASharp(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
                        ~ASharp();

               void         update(void); 
       virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	   virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual bool         configure(void) ;                 /// Start graphical user interface        
};



// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ASharp,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_SHARPNESS,            // Category
                        "asharp",            // internal name (must be uniq!)
                        "asharp",            // Display name
                        QT_TR_NOOP("Adaptative sharpener by MarcFD.") // Description
                    );

void asharp_run_c(      uc* planeptr, int pitch,
                                        int height, int width, 
                                        int     T,int D, int B, int B2, bool bf );
/**
    \fn ASharp 
    \brief ctor
*/
ASharp::ASharp(ADM_coreVideoFilter *in,CONFcouple *couples) : ADM_coreVideoFilter(in,couples)
{
        if(!couples || !ADM_paramLoad(couples,asharp_param,&_param))
		{
            // Default value
            _param.t=2;
            _param.d=4;
            _param.b=-1;
            _param.bf=false;
        }
        update();
        original=new ADMImageDefault(info.width,info.height);
}
/**
    \fn dtor 
    \brief recompute parameters
*/

ASharp::~ASharp(void)
{
        if(original) delete original;     
        original=NULL;
}
/**
    \fn update 
    \brief recompute parameters
*/

void ASharp::update( void)
{
                // parameters floating point to fixed point convertion
                T = (int)(_param.t*(4<<7));
                D = (int)(_param.d*(4<<7));
                B = (int)(256-_param.b*64);
                B2= (int)(256-_param.b*48);

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

extern uint8_t DIA_getASharp(asharp *param, ADM_coreVideoFilter *in);

/**
    \fn configure
*/
bool ASharp::configure(void)
{
uint8_t r=0;
#if 0
        if( DIA_getASharp(&_param, previousFilter))
        {
                r=1;
        }
        update();
        return r;
#else
    return true;
#endif
}
/**
    \fn getConfiguration
*/
const char   *ASharp::getConfiguration(void)   
{
    static char s[256];
    snprintf(s,255,"Asharp by MarcFd");
    return s;
        
}
/**
    \fn getNextFrame
*/
 bool         ASharp::getNextFrame(uint32_t *fn,ADMImage *image)
{
ADMImage *src;
ADMImage *dst;
        dst=image;
        src=original;

        if(!previousFilter->getNextFrame(fn,src)) return false;

        dst->duplicate(src);
        asharp_run_c(     dst->GetWritePtr(PLANAR_Y),
                        dst->GetPitch(PLANAR_Y), 
                        info.height,
                        info.width,
                        T,
                        D,
                        B,
                        B2,
                        _param.bf);
        return 1;
}

/**
    \fn asharp_run_c
*/
void asharp_run_c(      uc* planeptr, int pitch,
                                        int height, int width, 
                                        int     T,int D, int B, int B2, bool bf )
        {
                uc* lineptr = new uc[width];
                int p = pitch;
                int y,x;
                uc* cfp = planeptr+pitch;
                uc* lp = lineptr;
                memcpy(lp,planeptr,width);
                for (y=1;y<height-1;y++) {
                        int last = cfp[0];
                        for (x=0;x<width-1;x++) {

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

                                #define CHECK(A) \
                                if (abs(A-cfp[x])>dev) dev = abs(A-cfp[x]);
                                
                                if (bf) {

                                if (y%8>0) {
                                        if (x%8>0) CHECK(lp[x-1])
                                        CHECK(lp[x  ])
                                        if (x%8<7) CHECK(lp[x+1])
                                }
                                if (x%8>0) CHECK(last)
                                if (x%8<7) CHECK(cfp[x  +1])
                                if (y%8<7) {
                                        if (x%8>0) CHECK(cfp[x+p-1])
                                        CHECK(cfp[x+p  ])
                                        if (x%8<7) CHECK(cfp[x+p+1])
                                }

                                } else {
                                        CHECK(lp[x-p-1])
                                        CHECK(lp[x-p  ])
                                        CHECK(lp[x-p+1])
                                        CHECK(last)
                                        CHECK(cfp[x  +1])
                                        CHECK(cfp[x+p-1])
                                        CHECK(cfp[x+p  ])
                                        CHECK(cfp[x+p+1])
                                }

                                T2 = T;
                                diff = cfp[x]-avg;
                                int D2 = D;

                                if (x%8==6) D2=(D2*B2)>>8;
                                if (x%8==7) D2=(D2*B)>>8;
                                if (x%8==0) D2=(D2*B)>>8;
                                if (x%8==1) D2=(D2*B2)>>8;
                                if (y%8==6) D2=(D2*B2)>>8;
                                if (y%8==7) D2=(D2*B)>>8;
                                if (y%8==0) D2=(D2*B)>>8;
                                if (y%8==1) D2=(D2*B2)>>8;

                                int Da = -32+(D>>7);
                                if (D>0) T2 = ((((dev<<7)*D2)>>16)+Da)<<4;

                                if (T2>T) T2=T;
                                if (T2<-32) T2=-32;

                                tmp = (((diff<<7)*T2)>>16)+cfp[x];

                                if (tmp < 0) tmp = 0;
                                if (tmp > 255) tmp = 255;
                                lp[x-1] = last;
                                last = cfp[x];
                                cfp[x] = tmp;
                        }
                        lp[x] = cfp[x];
                        cfp += pitch;
                }
        delete[] lineptr;
} 
//**********************************
