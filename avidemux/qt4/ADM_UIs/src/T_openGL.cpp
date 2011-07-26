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
    glProgram=NULL;
    fboY=NULL;
    fboUV=NULL;
    firstRun=true;
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
    // glTexture TODO

}
/**
    \fn dtor
*/
ADM_coreVideoFilterQtGl::~ADM_coreVideoFilterQtGl()
{
    ADM_info("Gl filter : Destroying..\n");
    if(glProgram) delete glProgram;
    glProgram=NULL;
    if(fboY) delete fboY;
    fboY=NULL;
    if(fboUV) delete fboUV;
    fboUV=NULL;

}
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
        const uchar *src=qimg.constScanLine(height-y);
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
// EOF