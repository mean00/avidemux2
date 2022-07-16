/** *************************************************************************
                    \fn       rotateFilter.cpp  
                    \brief simplest of all video filters, it does nothing

    copyright            : (C) 2009 by mean
                               2022 by szlldm

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_factory.h"
#include "ADM_threads.h"
#include "rte.h"
#include "rte_desc.cpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
    \class arbitraryRotate
*/
class arbitraryRotate
{
protected:
    int         initialized;
    uint32_t    threads;
    int         _iw,_ih,_ow,_oh;
    float       lastAngle;
    bool        doEcho;
    int         greatSize;
    ADMImage    *greatBuf;
    ADMImage    *echo;
    ADMColorScalerFull * resizerOrigToEcho;
    ADMColorScalerFull * resizerEchoToGreat;
    int         **pixelIndex;
    int         **weight;
    pthread_t   *worker_threads;

    typedef struct {
        uint32_t    w,h;
        ADMImage    *src;
        ADMImage    *dst;
        int         **pixelIndex;
        int         **weight;        
        uint32_t    ystart, yincr, plane;
    } worker_thread_arg;
    
    worker_thread_arg *worker_thread_args;

    static void *worker_thread( void *ptr );
    
public:
                arbitraryRotate(int w, int h);
                ~arbitraryRotate();
    void        reconfig(float angle, int padding);
    void        rotate(ADMImage *source,ADMImage *target);
    void        getOutputSize(int iw, int ih, int * ow, int * oh);
};

arbitraryRotate::arbitraryRotate(int w, int h)
{
    initialized = 0;
    ADM_assert(_iw = w);
    ADM_assert(_ih = h);
    double hw = w/2.0;
    double hh = h/2.0;
    greatSize = std::sqrt(hw*hw + hh*hh)*2.0*std::sqrt(2.0);
    greatSize = ((greatSize+3)/4)*4;    // round up to multiple of 4
    
    greatBuf = NULL;
    echo = NULL;
    resizerOrigToEcho = NULL;
    resizerEchoToGreat = NULL;
    pixelIndex = NULL;
    weight = NULL;
    
    threads = ADM_cpu_num_processors();
    threads /= 2;
    threads += 1;
    if (threads < 1)
        threads = 1;

    worker_threads = new pthread_t [threads];
    worker_thread_args = new worker_thread_arg [threads];
}

arbitraryRotate::~arbitraryRotate()
{
    if (greatBuf) delete greatBuf;
    if (echo) delete echo;
    if (resizerOrigToEcho) delete resizerOrigToEcho;
    if (resizerEchoToGreat) delete resizerEchoToGreat;
    if (pixelIndex)
    {
        delete [] pixelIndex[0];
        delete [] pixelIndex[1];
        delete [] pixelIndex[2];
        delete [] pixelIndex;
    }
    if (weight)
    {
        delete [] weight[0];
        delete [] weight[1];
        delete [] weight[2];
        delete [] weight;
    }
    
    delete [] worker_threads;
    delete [] worker_thread_args;
}

void arbitraryRotate::reconfig(float angle, int padding)
{
    angle -= std::floor(angle/360.0)*360.0;
    doEcho = (padding == 1);
    
    if (doEcho)
    {
        if (echo == NULL)
        {
            echo = new ADMImageDefault(16,16);
        }
        if (resizerOrigToEcho == NULL)
        {
            resizerOrigToEcho=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                        _iw, _ih, 
                        16,16,
                        ADM_PIXFRMT_YV12,ADM_PIXFRMT_YV12);
        }
        if (resizerEchoToGreat == NULL)
        {
            resizerEchoToGreat=new ADMColorScalerFull(ADM_CS_LANCZOS, 
                        16,16, 
                        greatSize,greatSize,
                        ADM_PIXFRMT_YV12,ADM_PIXFRMT_YV12);            
        }
    }
    if (greatBuf == NULL)
    {
        greatBuf = new ADMImageDefault(greatSize,greatSize);
        greatBuf->blacken();
    }
    
    if (initialized==0 || lastAngle!=angle)
    {
        lastAngle = angle;
        
        double ccos = std::cos(M_PI * angle / 180.0);
        double csin = std::sin(M_PI * angle / 180.0);
        double p, minx, miny, maxx, maxy;
        minx = miny = maxx = maxy = 0;
        p = (_iw-1)*ccos;
        if (p < minx) minx = p;
        if (p > maxx) maxx = p;
        p = (_iw-1)*csin;
        if (p < miny) miny = p;
        if (p > maxy) maxy = p;
        p = (_ih-1)*csin*-1.0;
        if (p < minx) minx = p;
        if (p > maxx) maxx = p;
        p = (_ih-1)*ccos;
        if (p < miny) miny = p;
        if (p > maxy) maxy = p;
        p = (_iw-1)*ccos - (_ih-1)*csin;
        if (p < minx) minx = p;
        if (p > maxx) maxx = p;
        p = (_iw-1)*csin + (_ih-1)*ccos;
        if (p < miny) miny = p;
        if (p > maxy) maxy = p;
        double nw = maxx - minx + 1;
        double nh = maxy - miny + 1;
        
        _ow = nw+0.5;
        _ow = ((_ow+1)/2)*2;
        _oh = nh+0.5;
        _oh = ((_oh+1)/2)*2;

        if (pixelIndex)
        {
            delete [] pixelIndex[0];
            delete [] pixelIndex[1];
            delete [] pixelIndex[2];
            delete [] pixelIndex;
        }
        if (weight)
        {
            delete [] weight[0];
            delete [] weight[1];
            delete [] weight[2];
            delete [] weight;
        }
        
        pixelIndex = new int* [3];
        pixelIndex[0] = new int [2* _ow * _oh];
        pixelIndex[1] = new int [2* (_ow/2) * (_oh/2)];
        pixelIndex[2] = new int [2* (_ow/2) * (_oh/2)];
        weight = new int* [3];
        weight[0] = new int [2* _ow * _oh];
        weight[1] = new int [2* (_ow/2) * (_oh/2)];
        weight[2] = new int [2* (_ow/2) * (_oh/2)];
        
        int pw = _ow;
        int ph = _oh;
        int pg = greatSize;
        for (int p=0; p<3; p++)
        {
            if (p==1)
            {
                pw /= 2;
                ph /= 2;
                pg /= 2;
            }
            
            int spitch = greatBuf->GetPitch((ADM_PLANE)p);
            
            double centerX, centerY, centerG;
            centerX = (pw-1)/2.0;
            centerY = (ph-1)/2.0;
            centerG = (pg-1)/2.0;
            for (int y=0; y<ph; y++)
            {
                for (int x=0; x<pw; x++)
                {
                    double dX, dY, sX, sY, mX, mY;
                    dX = x - centerX;
                    dY = y - centerY;
                    sX = dX*std::cos(M_PI * angle / -180.0) - dY*std::sin(M_PI * angle / -180.0);
                    sY = dX*std::sin(M_PI * angle / -180.0) + dY*std::cos(M_PI * angle / -180.0);
                    sX += centerG;
                    sY += centerG;
                    mX = sX - std::floor(sX);
                    mY = sY - std::floor(sY);
                    
                    int srcX1, srcY1, srcY2, m1, m2;
                    srcX1 = std::floor(sX);
                    srcY1 = std::floor(sY);
                    srcY2 = srcY1 + 1;
                    m1 = std::round(mX * 256.0);
                    m2 = std::round(mY * 256.0);
                    
                    if (srcX1 < 0) srcX1 = 0;
                    if (srcY1 < 0) srcY1 = 0;
                    if (srcY2 < 0) srcY2 = 0;
                    
                    if (srcX1 >= pg) srcX1 = (pg-1);
                    if (srcY1 >= pg) srcY1 = (pg-1);
                    if (srcY2 >= pg) srcY2 = (pg-1);
                    
                    if (m1 < 0) m1 = 0;
                    if (m2 < 0) m2 = 0;
                    if (m1 > 256) m1 = 256;
                    if (m2 > 256) m2 = 256;
                    
                    pixelIndex[p][(y*pw + x)*2 + 0] = srcY1*spitch + srcX1;
                    pixelIndex[p][(y*pw + x)*2 + 1] = srcY2*spitch + srcX1;


                    weight[p][(y*pw + x)*2 + 0] = m1;
                    weight[p][(y*pw + x)*2 + 1] = m2;
                }
            }
        }
    }
    
    initialized = 1;
    
}

void *arbitraryRotate::worker_thread( void *ptr )
{
    worker_thread_arg * arg = (worker_thread_arg*)ptr;
    
    int pw = arg->w;
    int ph = arg->h;
    int p = arg->plane;
    {
        uint8_t * src = arg->src->GetWritePtr((ADM_PLANE)p);
        int dpitch = arg->dst->GetPitch((ADM_PLANE)p);
        for (int y=arg->ystart; y<ph; y+=arg->yincr)
        {
            uint8_t * dst = arg->dst->GetWritePtr((ADM_PLANE)p);
            dst += y*dpitch;
            int *pixIndex = arg->pixelIndex[p];
            int *wght = arg->weight[p];
            pixIndex += (2*y*pw);
            wght   += (2*y*pw);

            for (int x=0; x<pw; x++)
            {
                int a,b,m,n;
                
                m = pixIndex[0];
                n = m+1;
                a = src[m]*256 + (src[n] - src[m])*wght[0];
                m = pixIndex[1];
                n = m+1;
                b = src[m]*256 + (src[n] - src[m])*wght[0];
                
                dst[x] = (a*256 + (b-a)*wght[1])/65536;
                
                pixIndex += 2;
                wght += 2;
            }
        }    
    }
    
    pthread_exit(NULL);
    return NULL;
}

void arbitraryRotate::rotate(ADMImage *source,ADMImage *target)
{
    ADM_assert(initialized);
    ADM_assert(source->_width == _iw);
    ADM_assert(source->_height == _ih);
    ADM_assert(target->_width == _ow);
    ADM_assert(target->_height == _oh);
    
    if (doEcho)
    {
        resizerOrigToEcho->convertImage(source,echo);
        for (int p=0; p<3; p++)
        {
            uint8_t * ptr = echo->GetWritePtr((ADM_PLANE)p);
            uint32_t stride = echo->GetPitch((ADM_PLANE)p);
            int d = ((p>0) ? 8:16);
            int bias = ((p>0) ? 2:3);
            int biasX=((_iw>_ih) ? 0:bias);
            int biasY=((_iw>_ih) ? bias:0);
            for (int y=1; y<(d-1); y++)
            {
                int sy = ((y<(d/2))?0:(d-1));
                for (int x=1; x<(d-1); x++)
                {
                    int sx = ((x<(d/2))?0:(d-1));
                    if ((abs(x-d/2)+biasX)<(abs(y-d/2)+biasY))
                    {
                        ptr[y*stride+x] = ptr[sy*stride+x];
                    }
                    else
                    {
                        ptr[y*stride+x] = ptr[y*stride+sx];
                    }
                }
            }
        }
        resizerEchoToGreat->convertImage(echo,greatBuf);
    }
    else
    {
        greatBuf->blacken();
    }
    source->copyTo(greatBuf, (greatSize-_iw)/2, (greatSize-_ih)/2);
   
    for (int p=0; p<3; p++)
    {
        for (int tr=0; tr<threads; tr++)
        {
            worker_thread_args[tr].plane = p;
            worker_thread_args[tr].w = ((p==0) ? _ow : (_ow/2));
            worker_thread_args[tr].h = ((p==0) ? _oh : (_oh/2));
            worker_thread_args[tr].src = greatBuf;
            worker_thread_args[tr].dst = target;
            worker_thread_args[tr].pixelIndex = pixelIndex;
            worker_thread_args[tr].weight = weight;
            worker_thread_args[tr].ystart = tr;
            worker_thread_args[tr].yincr = threads;
        }

        for (int tr=0; tr<threads; tr++)
        {
            pthread_create( &worker_threads[tr], NULL, worker_thread, (void*) &worker_thread_args[tr]);
        }

        // work in thread workers...

        for (int tr=0; tr<threads; tr++)
        {
            pthread_join( worker_threads[tr], NULL);
        }
    }
  
}

void arbitraryRotate::getOutputSize(int iw, int ih, int * ow, int * oh)
{
    ADM_assert(initialized);
    ADM_assert(iw == _iw);
    ADM_assert(ih == _ih);
    *ow = _ow;
    *oh = _oh;
}

/**
    \class rotateFilter
*/
class rotateFilter : public  ADM_coreVideoFilter
{
protected:
        rte                  param;
        bool                 reset(void);
        ADMImage             *src;
        arbitraryRotate      *arbRot;
public:
                    rotateFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~rotateFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;     /// Start graphical user interface
private:
        void                 rotatePlane(uint32_t angle,uint8_t *src, uint32_t srcPitch, uint8_t *dst, uint32_t dstPitch, uint32_t width, uint32_t height);
        void                 do_rotate(ADMImage *source,ADMImage *target,uint32_t angle);
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   rotateFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "aarotate",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("aarotate","Arbitrary Rotate"),            // Display name
                        QT_TRANSLATE_NOOP("aarotate","Rotate the image by arbitrary angle.") // Description
                    );

/**
    \fn reset
*/
bool rotateFilter::reset(void)
{
    uint32_t w=previousFilter->getInfo()->width;
    uint32_t h=previousFilter->getInfo()->height;
    if (param.angle == 0 || param.angle == 180)
    {
        info.width=w;
        info.height=h;
    } else
    if (param.angle == 90 || param.angle == 270)
    {
        info.width=h;
        info.height=w;
    } else
    {
        int ow, oh;
        arbRot->reconfig(param.angle, param.pad);
        arbRot->getOutputSize(w, h, &ow, &oh);
        info.width=ow;
        info.height=oh;        
    }
    
    return true;

}
/**
    \fn rotateFilter
    \brief constructor
*/
rotateFilter::rotateFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    src=NULL;
    arbRot=NULL;
    param.angle=0;
    param.pad=0;
    if(setup && !ADM_paramLoadPartial(setup,rte_param,&param))
    {
        // Default value
        param.angle=0;// Bff=0 / 1=tff
        param.pad=0;
    }
    param.angle -= std::floor(param.angle/360.0)*360.0;
    src=new ADMImageDefault(previousFilter->getInfo()->width,previousFilter->getInfo()->height);
    arbRot = new arbitraryRotate(previousFilter->getInfo()->width,previousFilter->getInfo()->height);
    reset();
}
/**
    \fn rotateFilter
    \brief destructor
*/
rotateFilter::~rotateFilter()
{
      if(src) delete src;
      src=NULL;
      if (arbRot) delete arbRot;
      arbRot=NULL;
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool rotateFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,src))
    {
        ADM_warning("rotate : Cannot get frame\n");
        return false;
    }
    if (param.angle == 0 || param.angle == 180 || param.angle == 90 || param.angle == 270)
    {
        do_rotate(src,image,param.angle);
    } else
    {
        ADM_assert(arbRot);
        arbRot->rotate(src,image);
    }
    image->copyInfo(src);
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         rotateFilter::getCoupledConf(CONFcouple **couples)
{
   return ADM_paramSave(couples, rte_param,&param);
}

void rotateFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, rte_param, &param);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *rotateFilter::getConfiguration(void)
{
    static char buffer[80];
    if (std::floor(param.angle) == param.angle)
    {
        snprintf(buffer,80,"Rotate by %.0f degrees.",param.angle);
    }
    else
    {
        snprintf(buffer,80,"Rotate by %.3f degrees.",param.angle);
    }
    return buffer;
}
/**
    \fn rotatePlane
*/
void rotateFilter::rotatePlane(uint32_t angle,uint8_t *src,      uint32_t srcPitch, 
                 uint8_t *dst,  uint32_t dstPitch, uint32_t width,    uint32_t height)
{
    int32_t dstIncPix, dstIncLine;
    
    switch(angle)
    {
        case 0:   BitBlit(dst,dstPitch,src,srcPitch,width,height);return;break;
        case 180: dstIncPix=-1;       dstIncLine=-dstPitch; dst=dst+((height-1)*dstPitch)+width-1;break;

        case 90:  dstIncPix=dstPitch; dstIncLine=-1;        dst=dst+height-1;break;
        case 270: dstIncPix=-dstPitch;dstIncLine=1;         dst=dst+((width-1)*dstPitch);break;
        default:
            break;
    }
    
    uint8_t *lineIn,*lineOut;
    for(int y=0;y<height;y++)
    {
        lineIn=src+srcPitch*y;
        lineOut=dst+dstIncLine*y;
        for(int x=0;x<width;x++)
        {
            *lineOut=*lineIn;
            lineIn++;
            lineOut+=dstIncPix;
        }
    }


}

/**
    \fn do_rotate
*/
void rotateFilter::do_rotate(ADMImage *source,ADMImage *target,uint32_t angle)
{
uint8_t *in,*out;
uint32_t width,height;
uint32_t srcPitch,dstPitch;

    for(int i=0;i<3;i++)
    {
         ADM_PLANE plane=(ADM_PLANE)i;
         width=source->_width;
         height=source->_height;
         if(i)
            {
                width>>=1;
                height>>=1;
            }
          in=source->GetReadPtr(plane);
          srcPitch=source->GetPitch(plane);
          dstPitch=target->GetPitch(plane);
          out=target->GetWritePtr(plane);
          rotatePlane(angle,in,  srcPitch, out,  dstPitch,  width,  height);
    }
}

/**
    \fn configure
*/
bool rotateFilter::configure( void)
{
    diaMenuEntry paddingValues[]={
        {0,QT_TRANSLATE_NOOP("aarotate","Black"),NULL},
        {1,QT_TRANSLATE_NOOP("aarotate","Echo"),NULL}
    };
    
    ELEM_TYPE_FLOAT customAngle = param.angle;
    
    diaElemFloat    arbAngle(&customAngle,QT_TRANSLATE_NOOP("aarotate","_Angle:"),0,360,NULL,3);
    diaElemMenu     padding(&(param.pad),QT_TRANSLATE_NOOP("aarotate","Padding:"),2,paddingValues,NULL);
    
    diaElem *allWidgets[]={&arbAngle, &padding};
    if( !diaFactoryRun(QT_TRANSLATE_NOOP("aarotate","Rotate"),2,allWidgets)) return false;

    param.angle = customAngle;
    param.angle -= std::floor(param.angle/360.0)*360.0;

    reset();
    return true;
}
//EOF
