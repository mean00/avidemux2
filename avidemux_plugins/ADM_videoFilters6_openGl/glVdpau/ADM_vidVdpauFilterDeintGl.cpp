/**
    \brief VDPAU filters Deinterlacer
    \author mean (C) 2010
  
    This version uses openGL to convert the output surface to YV12


*/


#define __STDC_CONSTANT_MACROS
#define GL_GLEXT_PROTOTYPES

#       include <GL/gl.h>
#       include <GL/glu.h>
#       include <GL/glext.h>

#include <QtGui/QImage> 
#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLShader>
#include <list>

#include "ADM_coreConfig.h"
#ifdef USE_VDPAU
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/vdpau.h"
}

#define ADM_LEGACY_PROGGY
#include "ADM_default.h"

#include "ADM_coreVideoFilterInternal.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"

#include "T_openGL.h"
#include "T_openGLFilter.h"


#include "vdpauFilterDeint.h"
#include "ADM_vidVdpauFilterDeint.h"
#include "ADM_coreVdpau/include/ADM_coreVdpau.h"
//
//#define DO_BENCHMARK
#define NB_BENCH 100

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif



static const char *glShaderRgb =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTextureY;\n" // tex unit 0
    "uniform mat4 metrix;\n"
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"
    "const vec4 offset=vec4(0,0.5,0.5,0);\n"
	"void main(void) {\n"
    "  float nx = gl_TexCoord[0].x;\n"
	"  float ny = gl_TexCoord[0].y;\n"
	"  vec4 texin = texture2DRect(myTextureY, vec2(nx,ny));\n" 
    "  vec4 texout;\n"
    "  texout=metrix*texin ;\n"
    "  texout=texout+offset;\n"
    "  gl_FragColor=texout;"
	"}\n";

//--------------------------------------------------------------------------------------
/***/


/***/
PFNGLVDPAUINITNVPROC                VDPAUInitNV=NULL;
PFNGLVDPAUFININVPROC                VDPAUFiniNV=NULL;

PFNGLVDPAUREGISTEROUTPUTSURFACENVPROC VDPAURegisterOutputSurfaceNV=NULL;
PFNGLVDPAUREGISTERVIDEOSURFACENVPROC  VDPAURegisterVideoSurfaceNV=NULL;
PFNGLVDPAUUNREGISTERSURFACENVPROC     VDPAUUnregisterSurfaceNV=NULL;

PFNGLVDPAUMAPSURFACESNVPROC         VDPAUMapSurfacesNV=NULL;
PFNGLVDPAUUNMAPSURFACESNVPROC       VDPAUUnmapSurfacesNV=NULL;
PFNGLVDPAUSURFACEACCESSNVPROC       VDPAUSurfaceAccessNV=NULL;


/***/
#define GETFUNC(x)   x = (typeof(x) )ADM_getGlWidget()->context()->getProcAddress(QLatin1String("gl"#x));\
    if(!x) \
    {\
            ADM_error("Cannot get "#x"\n");\
            exit(-1);\
    }


static bool vdpauGlInited=false;
/**
    \fn processError
*/
static bool processError(const char *e)
{
    int x=glGetError();
    if(x!=GL_NO_ERROR)
    {
        ADM_error("%s: Error : %d %s\n",e,x,gluErrorString(x));
        return false;
    }
    return true;
}
/**
    \fn initGl
*/
bool vdpauVideoFilterDeint::initOnceGl(void)
{
    ADM_info("Initializing VDPAU<->openGl\n");
    if(vdpauGlInited)
    {
        ADM_info("Already done..\n");
        return true;
    }
    GETFUNC(VDPAUInitNV);
    GETFUNC(VDPAURegisterOutputSurfaceNV);
    GETFUNC(VDPAURegisterVideoSurfaceNV);
    GETFUNC(VDPAUUnregisterSurfaceNV);  
    GETFUNC(VDPAUMapSurfacesNV);  
    GETFUNC(VDPAUUnmapSurfacesNV);  
    GETFUNC(VDPAUSurfaceAccessNV);  
    GETFUNC(VDPAUFiniNV);
    const void *device=admVdpau::getVdpDevice();
    const void *proc=admVdpau::getProcAddress();
    ADM_info("VDPAU InitNv with device=%lx, proc=%lx\n",(long int)device,(long int)proc);
    VDPAUInitNV(device,proc);
    if(false==processError("InitNv")) return false;
    vdpauGlInited=true;
    return true;
}
/**
     \fn initGl
*/
bool vdpauVideoFilterDeint::initGl(void)
{
   initOnceGl();
   ADM_info("Creating VDPAU GL wrapper\n");
   rgb=new glRGB(this,NULL);
   //rgb->probe(outputSurface,NULL);
   return true;
}

/**
    \fn deInitGl
*/
bool vdpauVideoFilterDeint::deInitGl(void)
{
    ADM_info("Destroying VDPAU GL wrapper\n");
    delete rgb;
    rgb=NULL;
    return true;
}
/**
    \fn getResultSlow
*/
bool vdpauVideoFilterDeint::getResultSlow(ADMImage *image)
{
  
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceGetBitsNative(outputSurface,
                                                            tempBuffer, 
                                                            info.width,info.height))
    {
        ADM_warning("[Vdpau] Cannot copy back data from output surface\n");
        return false;
    }
  
    // tempBuffer                
    return rgb->imageToImage((const char *)tempBuffer,image);
}
/**
    \fn     getResult
    \brief  Convert the output surface into an ADMImage
*/
bool vdpauVideoFilterDeint::getResult(ADMImage *image)
{
    
    if(false==rgb->surfaceToImage(outputSurface,image))
            return getResultSlow(image);
    return true;
}
#else // USE_VDPAU
static void dummy_fun(void)
{
    return ;
}
#endif // use VDPAU
//-----------------------------------------------------------------
/**
    \fn render
*/
bool glRGB::render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo)
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
    \fn ctor
*/
glRGB::glRGB(ADM_coreVideoFilter *previous,CONFcouple *conf) 
            :  ADM_coreVideoFilterQtGl(previous,conf)
{
        widget->makeCurrent();
        fboY->bind();
        ADM_info("Compiling shader \n");
        glProgramY = new QGLShaderProgram(context);
        ADM_assert(glShaderRgb);
        if ( !glProgramY->addShaderFromSourceCode(QGLShader::Fragment, glShaderRgb))
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
        //
        memset(realMatrix,0,sizeof(realMatrix));
        realMatrix[0]=0.299;
        realMatrix[1]=0.587;
        realMatrix[2]=0.114;

        realMatrix[4]=-0.14713;
        realMatrix[5]=-0.28886;
        realMatrix[6]=+0.436;

        realMatrix[8]=+0.615;
        realMatrix[9]=-0.51499;
        realMatrix[10]=-0.10001;

}
/**
    \fn dtor
*/
glRGB::~glRGB()
{

}
/**

*/
bool         glRGB::getNextFrame(uint32_t *fn,ADMImage *image) {ADM_assert(0);return false;}
bool         glRGB::getCoupledConf(CONFcouple **couples) {ADM_assert(0);return false;};   
/**
    \fn probe
*/
bool   glRGB::probe(VdpOutputSurface surf,ADMImage *image)
{
    widget->makeCurrent();
    glPushMatrix();
    // size is the last one...
    fboY->bind();
    processError("Bind");
    GLvdpauSurfaceNV s=VDPAURegisterOutputSurfaceNV((GLvoid *)surf,GL_TEXTURE_2D,1,texName);
    VDPAUUnregisterSurfaceNV(s);
    processError("Unregister");
    fboY->release();
    firstRun=false;
    glPopMatrix();
    widget->doneCurrent();
}
/**
    \fn surfaceToImage
*/
bool   glRGB::surfaceToImage(VdpOutputSurface surf,ADMImage *image)
{
    bool r=true;
    widget->makeCurrent();
    glPushMatrix();
    fboY->bind();
    processError("Bind");
    glProgramY->setUniformValue("myTextureY", 0); 
    processError("setUniform myTexture0");
    QMatrix4x4 quadmat(realMatrix);
    glProgramY->setUniformValue("metrix",quadmat);
    processError("setUniform Matrix");
    myGlActiveTexture(GL_TEXTURE0);
    processError("Active Texture");
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, texName[0]); 
    processError("Bind Texture");
    //
    GLvdpauSurfaceNV s=VDPAURegisterOutputSurfaceNV((GLvoid *)surf,GL_TEXTURE_2D,1,texName);
    printf("Surface =%d, GlSurface=%x, texName : %d\n",(int)surf,(int)s,(int)texName[0]);
    if(false==processError("Register"))
    {
            r=false;
            goto skip;
    }
    VDPAUSurfaceAccessNV(s,GL_READ_ONLY);
    VDPAUMapSurfacesNV(1,&s);
    processError("Map");

    myGlActiveTexture(GL_TEXTURE0);
    processError("Active Texture");
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, texName[0]);  
    processError("Bind Texture");


    render(image,PLANAR_Y,fboY);
    downloadTextures(image,fboY);

    VDPAUUnmapSurfacesNV(1,&s);
    processError("Unmap");
    VDPAUUnregisterSurfaceNV(s);
    processError("Unregister");
skip:
    fboY->release();
    firstRun=false;
    glPopMatrix();
    widget->doneCurrent();
  

    return r;
}
/**
    \fn image2image
*/
bool glRGB::imageToImage(const char *buffer,ADMImage *image)
{
    bool r=true;
    int width=image->GetWidth(PLANAR_Y);
    int height=image->GetHeight(PLANAR_Y);
    widget->makeCurrent();
    glPushMatrix();
    // size is the last one...
    fboY->bind();
    processError("Bind");
    glProgramY->setUniformValue("myTextureY", 0); 
    processError("setUniform myTexture0");
    QMatrix4x4 quadmat(realMatrix);
    glProgramY->setUniformValue("metrix",quadmat);
    processError("setUniform Matrix");
    myGlActiveTexture(GL_TEXTURE0);
    processError("Active Texture");
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, texName[0]); 
    processError("Bind Texture");

    // upload image
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        if(!firstRun)
        {
            glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA, 
                            width,
                            height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                            buffer);
        }else
        {
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                width,
                height,
                GL_RGBA, GL_UNSIGNED_BYTE, 
                buffer);
        }
    //-----------------
    render(image,PLANAR_Y,fboY);
    downloadTextures(image,fboY);

    fboY->release();
    firstRun=false;
    glPopMatrix();
    widget->doneCurrent();
  

    return r;
}

//****************
// EOF
