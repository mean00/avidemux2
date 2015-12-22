/** *************************************************************************
                    \fn       glYadif Filter.cpp  
                    \brief    simple fragment shader

    Smoothing filter
    Do a 3x3 convolution on a image to get a mask
    Depending on the mask we let through or do convolution

    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_openGL.h"
#define ADM_LEGACY_PROGGY
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "T_openGLFilter.h"
#include "glYadif.h"
#include "ADM_clock.h"
#include "yadif.h"
#include "yadif_desc.cpp"
//************************************************
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define ABS(a) ((a) > 0 ? (a) : (-(a)))

#define MIN3(a,b,c) MIN(MIN(a,b),c)
#define MAX3(a,b,c) MAX(MAX(a,b),c)

static void filter_plane(int mode, uint8_t *dst, int dst_stride, const uint8_t *prev0, const uint8_t *cur0, const uint8_t *next0, int refs, int w, int h, int parity, int tff, int mmx);
/**

*/

//#define BENCH 1
//#define BENCH_READTEXTURE


/**
    \class glYadif
*/
class glYadif : public  ADM_coreVideoFilterQtGl
{
protected:
                    ADMImage    *original;

                    yadif       configuration;
                    void        updateInfo(void);
                    bool        getNextFrame2(uint32_t *fn,ADMImage *image);
protected:
                        bool render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo);
public:
                             glYadif(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~glYadif();

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;                        /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   glYadif,   // Class
                        1,0,0,              // Version
                        ADM_UI_QT4+ADM_FEATURE_OPENGL,         // UI
                        VF_OPENGL,            // Category
                        "glYadif",            // internal name (must be uniq!)
                        "Yadif (openGl)",            // Display name
                        "Yet another deinterlacer, using shaders." // Description
                    );

// Now implements the interesting parts
/**
    \fn glYadif
    \brief constructor
*/
glYadif::glYadif(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterQtGl(in,setup)
{
UNUSED_ARG(setup);
        widget->makeCurrent();
        fboY->bind();
        printf("Compiling shader \n");
        glProgramY = new QGLShaderProgram(context);
        ADM_assert(glProgramY);
        if ( !glProgramY->addShaderFromSourceCode(QGLShader::Fragment, myShaderY))
        {
                ADM_error("[GL Render] Fragment log: %s\n", glProgramY->log().toUtf8().constData());
                ADM_assert(0);
        }
        if ( !glProgramY->link())
        {
            ADM_error("[GL Render] Link log: %s\n", glProgramY->log().toUtf8().constData());
            ADM_assert(0);
        }

        if ( !glProgramY->bind())
        {
                ADM_error("[GL Render] Binding FAILED\n");
                ADM_assert(0);
        }

        fboY->release();
        widget->doneCurrent();

        original=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
        if(!setup || !ADM_paramLoad(setup,yadif_param,&configuration))
        {
            // Default value
            configuration.mode=0;
            configuration.order=1;
        }
        vidCache = new VideoCache (10, in);
        updateInfo();
        myName="yadif";


}
/**
    \fn glYadif
    \brief destructor
*/
glYadif::~glYadif()
{
        delete  original;
        original=NULL;
       
        delete vidCache;
        vidCache = NULL;
}
/**
    \fn updateInfo
*/

void glYadif::updateInfo(void)
{
   memcpy(&info,previousFilter->getInfo(),sizeof(info)); 
  if(configuration.mode &1 ) // Bob
  {
    info.totalDuration*=2;
    info.frameIncrement/=2;
  }
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool glYadif::getNextFrame(uint32_t *fn,ADMImage *image)
{
    ADMImage *src, *dst, * prev, *next;
    int mode = configuration.mode;
    int n;
    if (mode & 1) 
                n = (nextFrame>>1); // bob
    else
                n = nextFrame;

    src = vidCache->getImage(n);
    *fn=n;
    if(!src) return false;  
    // If possible get previous image...
    if (n>0)
            prev =  vidCache->getImage( n-1); // get previous frame
    else
            prev= src; // get very first frame

    ADM_assert(prev);
    next=vidCache->getImage(n+1);
    if(!next) next=src;
    ADM_assert(next);
    
    dst = image;
    dst->copyInfo(src);

    if(!prev || !src || !next)
    {
        printf("Failed to read frame for frame %u\n",nextFrame);
        vidCache->unlockAll();
        return false;
    }


    widget->makeCurrent();
    glPushMatrix();
    // size is the last one...
    fboY->bind();

    glProgramY->setUniformValue("myTexturePrev", 0); 
    glProgramY->setUniformValue("myTextureCur",  1); 
    glProgramY->setUniformValue("myTextureNext", 2); 
    glProgramY->setUniformValue("myWidth", image->GetWidth(PLANAR_Y)); 
    glProgramY->setUniformValue("myHeight", image->GetHeight(PLANAR_Y)); 

    int tff = configuration.order;	
    int parity = (mode & 1) ? (nextFrame & 1) ^ (1^tff) : (tff ^ 1); 
    glProgramY->setUniformValue("myParity", parity); 

    // upload the 3 Y Plane ..
    uploadOnePlane(prev,PLANAR_Y,GL_TEXTURE0+0,texName[0]);
    uploadOnePlane(src, PLANAR_Y,GL_TEXTURE0+1,texName[1]);
    uploadOnePlane(next,PLANAR_Y,GL_TEXTURE0+2,texName[2]);

    render(image,PLANAR_Y,fboY);

    downloadTexture(image,PLANAR_Y,fboY);

    fboY->release();
    firstRun=false;
    glPopMatrix();
    widget->doneCurrent();
    
    ADMImage::copyPlane(src,image,PLANAR_U);
    ADMImage::copyPlane(src,image,PLANAR_V);

    return true;
}

/**
    \fn render
*/
bool glYadif::render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo)
{
    int width=image->GetWidth(plane);
    int height=image->GetHeight(plane);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    //
    glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);
	glTexCoord2i(width, 0);
	glVertex2i(width, 0);
	glTexCoord2i(width, height);
	glVertex2i(width ,height);
	glTexCoord2i(0, height);
	glVertex2i(0, height);
	glEnd();	// draw cube background
    return true;
}
/**
    \fn updateInfo
*/
bool glYadif::configure( void) 
{
    
     diaMenuEntry tMode[]={
                             {0,      QT_TR_NOOP("Temporal & spatial check"),NULL},
                             {1,   QT_TR_NOOP("Bob, temporal & spatial check"),NULL},
                             {2,      QT_TR_NOOP("Skip spatial temporal check"),NULL},
                             {3,  QT_TR_NOOP("Bob, skip spatial temporal check"),NULL}
          };
     diaMenuEntry tOrder[]={
                             {0,      QT_TR_NOOP("Bottom field first"),NULL},
                             {1,   QT_TR_NOOP("Top field first"),NULL}
          };
  
     diaElemMenu mMode(&(configuration.mode),   QT_TR_NOOP("_Mode:"), 4,tMode);
     diaElemMenu morder(&(configuration.order),   QT_TR_NOOP("_Order:"), 2,tOrder);
     
     diaElem *elems[]={&mMode,&morder};
     
     if(diaFactoryRun(QT_TR_NOOP("yadif"),sizeof(elems)/sizeof(diaElem *),elems))
     {
        updateInfo();
        return 1;
     }
     return 0;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         glYadif::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, yadif_param,&configuration);
}

void glYadif::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, yadif_param, &configuration);
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *glYadif::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"yadif : mode=%d, order=%d\n",
                (int)configuration.mode, (int)configuration.order);
    return conf;
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
bool glYadif::getNextFrame2(uint32_t *fn,ADMImage *image)
{

        int mode;
        int parity;
        int tff;
        int iplane;
        int cpu;
        int n;
        ADMImage *src, *dst, * prev, *next;
        
    
        mode = configuration.mode;

        if (mode & 1) 
        {
                n = (nextFrame>>1); // bob
        }
        else
                n = nextFrame;

        src = vidCache->getImage(n);
        *fn=n;
        if(!src) return false;
        
  
        // If possible get previous image...
        if (n>0)
                prev =  vidCache->getImage( n-1); // get previous frame
        else
                prev= src; // get very first frame

        ADM_assert(prev);
        next=vidCache->getImage(n+1);
        if(!next) next=src;
        ADM_assert(next);
        
        dst = image;
        dst->copyInfo(src);

        if(!prev || !src || !next)
        {
            printf("Failed to read frame for frame %u\n",nextFrame);
            vidCache->unlockAll();
            return 0;
        }
        
  // Construct a frame based on the information of the current frame
  // contained in the "vi" struct.
#if 0 //MEANX
        if (configuration.order == -1)
//		tff = avs_is_tff(&p->vi) == 0 ? 0 : 1; // 0 or 1
                tff = avs_get_parity(p->child, n) ? 1 : 0; // 0 or 1
        else
#endif
                tff = configuration.order;	
        
        parity = (mode & 1) ? (nextFrame & 1) ^ (1^tff) : (tff ^ 1);  // 0 or 1

      //MEANX  cpu = avs_get_cpu_flags(p->env);

        for (iplane = 0; iplane<3; iplane++)
        {
                ADM_PLANE plane = (iplane==0) ? PLANAR_Y : (iplane==1) ? PLANAR_U : PLANAR_V;

                const unsigned char* srcp = src->GetWritePtr(plane);
          // Request a Read pointer from the current source frame

                const unsigned char* prevp0 = prev->GetWritePtr( plane);
                unsigned char* prevp = (unsigned char*) prevp0; // with same pitch
          // Request a Read pointer from the prev source frame.

                const unsigned char* nextp0 = next->GetWritePtr( plane);
                unsigned char* nextp = (unsigned char*) nextp0; // with same pitch
          // Request a Read pointer from the next source frame.

                unsigned char* dstp = dst->GetWritePtr( plane);
                // Request a Write pointer from the newly created destination image.
          // You can request a writepointer to images that have just been

                const int dst_pitch = dst->GetPitch( plane);
          // Requests pitch (length of a line) of the destination image.
          // For more information on pitch see: http://www.avisynth.org/index.php?page=WorkingWithImages
                // (short version - pitch is always equal to or greater than width to allow for seriously fast assembly code)

                const int width =dst->GetPitch( plane);
          // Requests rowsize (number of used bytes in a line.
          // See the link above for more information.

                const int height = dst->GetHeight( plane);
          // Requests the height of the destination image.

                const int src_pitch = src->GetPitch(plane);
                const int prev_pitch = prev->GetPitch(plane);
                const int next_pitch = next->GetPitch(plane);

                // in v.0.1-0.3  all source pitches are  assumed equal (for simplicity)
                                // consider other (rare) case
                if (prev_pitch != src_pitch)
                {
                    prevp = (unsigned char *)ADM_alloc(height*src_pitch);
                    int h;
                    for (h=0; h<0; h++)
                      memcpy(prevp+h*src_pitch, prevp0+h*prev_pitch, width);
                }
                    
                if (next_pitch != src_pitch)
                {
                    nextp = (unsigned char *)ADM_alloc(height*src_pitch);
                    int h;
                    for (h=0; h<0; h++)
                      memcpy(nextp+h*src_pitch, nextp0+h*next_pitch, width);
                }
                    
                filter_plane(mode, dstp, dst_pitch, prevp, srcp, nextp, src_pitch, width, height, parity, tff, 0);
                if (prev_pitch != src_pitch)
                        ADM_dealloc(prevp);
                if (next_pitch != src_pitch)
                        ADM_dealloc(nextp);
        }
      vidCache->unlockAll();
      
      if (mode & 1) 
      {
            if(nextFrame&1)
                image->Pts+= info.frameIncrement;
      }
      //printf("out PTs=%"PRIu64", nextFrame=%d,inc=%d\n",image->Pts,(int)nextFrame,(int)info.frameIncrement);
      nextFrame++;
      
      return 1;
}

/**
    \fn filter_line_c
*/
static void filter_line_c(int mode, uint8_t *dst, const uint8_t *prev, const uint8_t *cur, const uint8_t *next, int w, int refs, int parity){
    int x;
    const uint8_t *prev2= parity ? prev : cur ;
    const uint8_t *next2= parity ? cur  : next;
    for(x=0; x<w; x++){
        int c= cur[-refs];
        int d= (prev2[0] + next2[0])>>1;
        int e= cur[+refs];
        int temporal_diff0= ABS(prev2[0] - next2[0]);
        int temporal_diff1=( ABS(prev[-refs] - c) + ABS(prev[+refs] - e) )>>1;
        int temporal_diff2=( ABS(next[-refs] - c) + ABS(next[+refs] - e) )>>1;
        int diff= MAX3(temporal_diff0>>1, temporal_diff1, temporal_diff2);
        int spatial_pred= (c+e)>>1;
        int spatial_score= ABS(cur[-refs-1] - cur[+refs-1]) + ABS(c-e)
                         + ABS(cur[-refs+1] - cur[+refs+1]) - 1;

#define CHECK(j)\
    {   int score= ABS(cur[-refs-1+ j] - cur[+refs-1- j])\
                 + ABS(cur[-refs  + j] - cur[+refs  - j])\
                 + ABS(cur[-refs+1+ j] - cur[+refs+1- j]);\
        if(score < spatial_score){\
            spatial_score= score;\
            spatial_pred= (cur[-refs  + j] + cur[+refs  - j])>>1;\

        CHECK(-1) CHECK(-2) }} }}
        CHECK( 1) CHECK( 2) }} }}

        if(mode<2){
            int b= (prev2[-2*refs] + next2[-2*refs])>>1;
            int f= (prev2[+2*refs] + next2[+2*refs])>>1;
#if 0
            int a= cur[-3*refs];
            int g= cur[+3*refs];
            int max= MAX3(d-e, d-c, MIN3(MAX(b-c,f-e),MAX(b-c,b-a),MAX(f-g,f-e)) );
            int min= MIN3(d-e, d-c, MAX3(MIN(b-c,f-e),MIN(b-c,b-a),MIN(f-g,f-e)) );
#else
            int max= MAX3(d-e, d-c, MIN(b-c, f-e));
            int min= MIN3(d-e, d-c, MAX(b-c, f-e));
#endif

            diff= MAX3(diff, min, -max);
        }

        if(spatial_pred > d + diff)
           spatial_pred = d + diff;
        else if(spatial_pred < d - diff)
           spatial_pred = d - diff;

        dst[0] = spatial_pred;

        dst++;
        cur++;
        prev++;
        next++;
        prev2++;
        next2++;
    }
}
/**
    \fn  filter_plane
*/
void filter_plane(int mode, uint8_t *dst, int dst_stride, const uint8_t *prev0, const uint8_t *cur0, const uint8_t *next0, int refs, int w, int h, int parity, int tff, int mmx)
{
void (*filter_line)(int mode, uint8_t *dst, const uint8_t *prev, const uint8_t *cur, const uint8_t *next, int w, int refs, int parity);
	int y;
	filter_line = filter_line_c;

        memcpy(dst, cur0, w);
        memcpy(dst + dst_stride, cur0 + refs, w);
        for(y=2; y<h-1; y++){
            if(((y ^ parity) & 1)){
                const uint8_t *prev= prev0 + y*refs;
                const uint8_t *cur = cur0 + y*refs;
                const uint8_t *next= next0 + y*refs;
                uint8_t *dst2= dst + y*dst_stride;
                filter_line(mode, dst2, prev, cur, next, w, refs, (parity ^ tff));
            }else{
                memcpy(dst + y*dst_stride, cur0 + y*refs, w);
            }
        }
        memcpy(dst + (h-1)*dst_stride, cur0 + (h-1)*refs, w);

}

//EOF
