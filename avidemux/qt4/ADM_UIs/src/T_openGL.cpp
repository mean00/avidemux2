    /***************************************************************************
  \file T_openGL.h
  \brief OpenGL related filters
  \author (C) 2011 Mean Fixounet@free.fr 
***************************************************************************/
#include "T_openGL.h"
#include "T_openGLFilter.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
static QGLWidget *thisWidget=NULL;
static GlActiveTexture_Type *myActiveTexture=NULL;


/**
    \fn ADM_setActiveTexture
*/
bool ADM_setActiveTexture(GlActiveTexture_Type *set)
{
    myActiveTexture=set;
    return true;
}
/**
    \fn ADM_getActiveTexture
*/
GlActiveTexture_Type *ADM_getActiveTexture(void)
{
    return myActiveTexture;
}

/**

*/
bool ADM_setGlWidget(QGLWidget *w)
{
        thisWidget=w;
        return true;
}
/**

*/
QGLWidget *ADM_getGlWidget(void)
{
        return thisWidget;
        
}
/**
    \fn ctor
*/
ADM_coreVideoFilterQtGl::ADM_coreVideoFilterQtGl(ADM_coreVideoFilter *previous,CONFcouple *conf)
:ADM_coreVideoFilter(previous,conf)
{
    context=NULL;
    glProgramY=NULL;
    glProgramUV=NULL;
    fboY=NULL;
    fboUV=NULL;
    widget=new QGLWidget(ADM_getGlWidget());
    widget->makeCurrent();
    firstRun=0;
    ADM_info("Gl : Allocating context and frameBufferObjects\n");
    context=QGLContext::currentContext();
    ADM_assert(context);
    fboY = new QGLFramebufferObject(info.width,info.height);
    ADM_assert(fboY);
    fboUV = new QGLFramebufferObject(info.width/2,info.height/2);
    ADM_assert(fboUV);
    myGlActiveTexture=ADM_getActiveTexture();
    if(!myGlActiveTexture)
    {
        ADM_error("Cannot get glActiveTexture\n");
        GUI_Error_HIG("","Cannot get glActiveTexture");
        ADM_assert(0);
    }
    glGenTextures(3,texName);
    widget->doneCurrent();
    // glTexture TODO

}
/**
    \fn dtor
*/
ADM_coreVideoFilterQtGl::~ADM_coreVideoFilterQtGl()
{
    ADM_info("Gl filter : Destroying..\n");
    glDeleteTextures(3,texName);
    if(glProgramY) delete glProgramY;
    glProgramY=NULL;
    if(glProgramUV) delete glProgramUV;
    glProgramUV=NULL;
    if(fboY) delete fboY;
    fboY=NULL;
    if(fboUV) delete fboUV;
    fboUV=NULL;
    if(widget) delete widget;       
    widget=NULL;
}
#if  1
#define TEX_Y_OFFSET 2
#define TEX_U_OFFSET 1
#define TEX_V_OFFSET 0  
#define TEX_A_OFFSET 3   
#else
#define TEX_Y_OFFSET 0
#define TEX_U_OFFSET 1
#define TEX_V_OFFSET 2   
#define TEX_A_OFFSET 3   
#endif
/**
    \fn downloadTexture
*/
bool ADM_coreVideoFilterQtGl::downloadTexture(ADMImage *image, ADM_PLANE plane,
        QGLFramebufferObject *fbo)
{
#ifdef BENCH_READTEXTURE
    {
    ADMBenchmark bench;
    for(int i=0;i<100;i++)
    {
        bench.start();
        QImage qimg(fbo->toImage());
        bench.end();
     }
    ADM_warning("convert to Qimage\n");
    bench.printResult();
    }
#endif

    QImage qimg(fbo->toImage()); // this is slow ! ~ 15 ms for a 720 picture (Y only).



    // Assume RGB32, read R or A
#ifdef BENCH_READTEXTURE
    ADMBenchmark bench;
    for(int i=0;i<100;i++)
    {
        bench.start();
#endif
    int stride=image->GetPitch(plane);
    uint8_t *to=image->GetWritePtr(plane);
    int width=image->GetWidth(plane);
    int height=image->GetHeight(plane);
    for(int y=0;y<height;y++)
    {
        const uchar *src=qimg.constScanLine(height-y-1);
        if(!src)
        {
            ADM_error("Can t get pointer to openGl texture\n");
            return false;
        }
        for(int x=0;x<width;x++)
            to[x]=src[x*4];
        to+=stride;
    }
#ifdef BENCH_READTEXTURE
        bench.end();
    }
    bench.printResult();

#endif
    return true;
}

/**
    \fn downloadTexture
    \brief Download YUVA texture into a YV12 image
*/
bool ADM_coreVideoFilterQtGl::downloadTextures(ADMImage *image,  QGLFramebufferObject *fbo)
{

    QImage qimg(fbo->toImage()); // this is slow ! ~ 15 ms for a 720 picture (Y only).
    // Assume RGB32, read R or A
    int strideY=image->GetPitch(PLANAR_Y);
    uint8_t *toY=image->GetWritePtr(PLANAR_Y);
    
    int width=image->GetWidth(PLANAR_Y);
    int height=image->GetHeight(PLANAR_Y);

    // Do Y
    for(int y=1;y<=height;y++)
    {
        const uchar *src=qimg.constScanLine(height-y);
        if(!src)
        {
            ADM_error("Can t get pointer to openGl texture\n");
            return false;
        }
#if 0
    printf("RGB");
    const uchar *src2=src;
    for(int i=0;i<10;i++)
    {
        printf("%d %d %d %d:",src2[i*4],src2[i*4+1],src2[i*4+2],src2[i*4+3]);
        src2+=4;
    }
    printf("\n");
#endif
        for(int x=0;x<width;x++)
        {
            toY[x]  = src[x*4+TEX_Y_OFFSET];
            
        }
        toY+=strideY;
    }
    // do U & V
    uint8_t *toU=image->GetWritePtr(PLANAR_U);
    uint8_t *toV=image->GetWritePtr(PLANAR_V);
    int      strideU=image->GetPitch(PLANAR_U);
    int      strideV=image->GetPitch(PLANAR_V);

    for(int y=1;y<=height;y+=2)
    {
        const uchar *src=qimg.constScanLine(height-y);
        if(!src)
        {
            ADM_error("Can t get pointer to openGl texture\n");
            return false;
        }
        for(int x=0;x<width;x+=2) // Stupid subsample: 1 out of 4
        {
            toU[x/2]  =  src[x*4+TEX_U_OFFSET];
            toV[x/2]  =  src[x*4+TEX_V_OFFSET];
        }
        toU+=strideU;
        toV+=strideV;
    }
    return true;
}
/**
    \fn uploadTexture
*/
void ADM_coreVideoFilterQtGl::uploadOnePlane(ADMImage *image, ADM_PLANE plane, GLuint tex,int texNum )
{
        myGlActiveTexture(tex);  // Activate texture unit "tex"
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, texNum); // Use texture "texNum"

        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        if(!firstRun)
        {
            glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, 
                            image->GetPitch(plane),
                            image->GetHeight(plane), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                            image->GetReadPtr(plane));
        }else
        {
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                image->GetPitch(plane),
                image->GetHeight(plane),
                GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                image->GetReadPtr(plane));
        }
}
/**
    \fn uploadTexture
*/
void ADM_coreVideoFilterQtGl::uploadAllPlanes(ADMImage *image)
{
          // Activate texture unit "tex"
        for(int xplane=2;xplane>=0;xplane--)
        {
            myGlActiveTexture(GL_TEXTURE0+xplane);
            ADM_PLANE plane=(ADM_PLANE)xplane;
            glBindTexture(GL_TEXTURE_RECTANGLE_NV, texName[xplane]); // Use tex engine "texNum"
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

                if(!firstRun)
                {
                    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, 
                                    image->GetPitch(plane),
                                    image->GetHeight(plane), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                                    image->GetReadPtr(plane));
                }else
                {
                    glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                        image->GetPitch(plane),
                        image->GetHeight(plane),
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                        image->GetReadPtr(plane));
                }
        }
}

// EOF