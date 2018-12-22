    /***************************************************************************
  \file T_openGL.h
  \brief OpenGL related filters
  \author (C) 2011 Mean Fixounet@free.fr 
***************************************************************************/
#include "ADM_openGl.h"
#include "T_openGLFilter.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
static QOpenGLWidget *thisWidget=NULL;

#define CHECK(x) if(!x) {GUI_Error_HIG("Missing extension "#x,#x" not defined");ADM_assert(0);}

/**

*/
bool ADM_setGlWidget(QOpenGLWidget *w)
{
    thisWidget=w;
    return true;
}
/**

*/
QOpenGLWidget *ADM_getGlWidget(void)
{
    return thisWidget;
}

/**
    \fn ctor
*/
ADM_coreVideoFilterQtGl::ADM_coreVideoFilterQtGl(ADM_coreVideoFilter *previous,CONFcouple *conf)
:ADM_coreVideoFilter(previous,conf),ADM_coreQtGl(ADM_getGlWidget())
{
    bufferARB=0;
    _parentQGL->makeCurrent();
    if(ADM_glHasARB())
            ADM_glExt::genBuffers(1,&bufferARB);        
    glProgramY=NULL;
    glProgramUV=NULL;
    fboY=NULL;
    fboUV=NULL;
    fboY = new QOpenGLFramebufferObject(info.width,info.height);
    ADM_assert(fboY);
    fboUV = new QOpenGLFramebufferObject(info.width/2,info.height/2);
    ADM_assert(fboUV);
    _parentQGL->doneCurrent();
}
/**
    \fn resizeFBO
*/
bool ADM_coreVideoFilterQtGl::resizeFBO(uint32_t w,uint32_t h)
{
    _parentQGL->makeCurrent();
    if(fboY) delete fboY;
    fboY=new QOpenGLFramebufferObject(w,h);
    _parentQGL->doneCurrent();
    checkGlError("resizeFBO");
    return true;
}
/**
    \fn dtor
*/
ADM_coreVideoFilterQtGl::~ADM_coreVideoFilterQtGl()
{
    ADM_info("Gl filter : Destroying..\n");
   
    if(glProgramY) delete glProgramY;
    glProgramY=NULL;
    if(glProgramUV) delete glProgramUV;
    glProgramUV=NULL;
    if(fboY) delete fboY;
    fboY=NULL;
    if(fboUV) delete fboUV;
    fboUV=NULL;
    if(ADM_glHasARB())
        ADM_glExt::deleteBuffers(1,&bufferARB);
    bufferARB=0;
}
/**
 * 
 * @param type
 * @param proggy
 * @return 
 */
QOpenGLShaderProgram *ADM_coreVideoFilterQtGl::createShaderFromSource(QOpenGLShader::ShaderType type,const char *proggy)
{
    QOpenGLShaderProgram *glProg = new QOpenGLShaderProgram(NULL);
    ADM_assert(glProg);
    if ( !glProg->addShaderFromSourceCode(type, proggy))
    {
            ADM_error("[GL Render] Fragment log: %s\n", glProg->log().toUtf8().constData());
            delete glProg;
            glProg=NULL;
            return NULL;
    }
    
    if ( !glProg->link())
    {
            ADM_error("[GL Render] Link log: %s\n", glProg->log().toUtf8().constData());
            delete glProg;
            glProg=NULL;
            return NULL;
    }

    if ( !glProg->bind())
    {
            ADM_error("[GL Render] Binding FAILED\n");
            delete glProg;
            glProg=NULL;
            return NULL;

    }

    return glProg;
}



// EOF
