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
#include "ADM_videoFilterDynamic.h"

#include "ADM_vidASharp_param.h"
class ASharp : public AVDMGenericVideoStream
{
private:
        
        VideoCache      *vidCache;
        ASHARP_PARAM    *_param;
        int32_t        T,D,B,B2;
        
public:    

                        ASharp(AVDMGenericVideoStream *in,CONFcouple *couples)   ;
                        ~ASharp();


                        void update(void);         
        char            *printConf( void );
        uint8_t         configure(AVDMGenericVideoStream *in);
        uint8_t         getCoupledConf( CONFcouple **couples);
        uint8_t         getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data,uint32_t *flags);
};


static FILTER_PARAM asharp_param={4,{"t", "b","d", "bf"}};


//REGISTERX(VF_SHARPNESS, "asharp",QT_TR_NOOP("asharp"),QT_TR_NOOP(""
//    "Adaptative sharpener by MarcFD."),VF_ASHARP,1,asharp_create,asharp_script);
VF_DEFINE_FILTER_UI(ASharp,asharp_param,
    asharp,
                                QT_TR_NOOP("asharp"),
                                1,
                                VF_SHARPNESS,
                                QT_TR_NOOP("Adaptative sharpener by MarcFD."));


//_______________________________________________

ASharp::ASharp(AVDMGenericVideoStream *in,CONFcouple *couples)

{
        _in=in;         
        memcpy(&_info,_in->getInfo(),sizeof(_info));    
        _info.encoding=1;
        _uncompressed=NULL;             
        _info.encoding=1;


        _param=new ASHARP_PARAM;
       
        if(couples)
                {
                     GET(t);
                     GET(d);
                     GET(b);
                     GET(bf); 
                }
                else // Default
                {
                        _param->t=2;
                        _param->d=4;
                        _param->b=-1;
                        _param->bf=0;
                }                              
                update();
                vidCache=new VideoCache(5,in);
}
//________________________________________________________
void ASharp::update( void)
{
                // parameters floating point to fixed point convertion
                T = (int)(_param->t*(4<<7));
                D = (int)(_param->d*(4<<7));
                B = (int)(256-_param->b*64);
                B2= (int)(256-_param->b*48);

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
//________________________________________________________
uint8_t ASharp::getCoupledConf( CONFcouple **couples)
{
        *couples=NULL;
        *couples=new CONFcouple(4);
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
               CSET(t);
               CSET(d);
               CSET(b);
               CSET(bf);    
        return 1;
}
//________________________________________________________
ASharp::~ASharp(void)
{

        if(vidCache) delete vidCache;     

     
        vidCache=NULL;

}

extern uint8_t DIA_getASharp(ASHARP_PARAM *param, AVDMGenericVideoStream *in);
//________________________________________________________
uint8_t ASharp::configure(AVDMGenericVideoStream *in)
{
uint8_t r=0;
        _in=in;
        if( DIA_getASharp(_param, _in))
        {
                r=1;
        }
        update();
        return r;
}

//________________________________________________________
char *ASharp::printConf( void )
{
       ADM_FILTER_DECLARE_CONF(" ASharp (MarcFD)");
        
}
        
//________________________________________________________
uint8_t ASharp::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                ADMImage *data,uint32_t *flags)
{
ADMImage *src;
ADMImage *dst;

        dst=data;
        if(frame>=_info.nb_frames) return 0;
        src=vidCache->getImage(frame);
        dst->duplicate(src);
        asharp_run_c(     dst->GetWritePtr(PLANAR_Y),
                        dst->GetPitch(PLANAR_Y), 
                        _info.height,
                        _info.width,
                        T,
                        D,
                        B,
                        B2,
                        _param->bf);

        vidCache->unlockAll();

        return 1;
}




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
