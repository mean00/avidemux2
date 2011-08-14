/**
    \brief VDPAU filters Deinterlacer
    \author mean (C) 2010
  
    This version uses openGL to convert the output surface to YV12


*/


#define __STDC_CONSTANT_MACROS
#define GL_GLEXT_PROTOTYPES

#       include <GL/gl.h>
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
#define ADM_INVALID_FRAME_NUM 0x80000000
#define ADM_NB_SURFACES 5

//#define DO_BENCHMARK
#define NB_BENCH 100

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

static int nbNvInit=0;
static const char *glShaderRgb =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTextureY;\n" // tex unit 0
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"
    
	"void main(void) {\n"
    "  float nx = gl_TexCoord[0].x;\n"
	"  float ny = gl_TexCoord[0].y;\n"
	"  vec4 texvalY = texture2DRect(myTextureY, vec2(nx,ny));\n" 
	"  gl_FragColor = vec4(texvalY.g, 128,128,1);\n" //texvalU.r, texvalV.r, 1.0);\n"
	"}\n";

//--------------------------------------------------------------------------------------
/***/
typedef GLintptr GLvdpauSurfaceNV;



typedef void            typeVDPAUInitNV(const void *vdpDevice,      const void *getProcAddress);
typedef GLvdpauSurfaceNV typeVDPAURegisterOutputSurfaceNV (const void *vdpSurface,
                                                 GLenum      target,
                                                 GLsizei     numTextureNames,
                                                 const GLuint *textureNames);
typedef void   typeVDPAUUnregisterSurfaceNV (GLvdpauSurfaceNV surface);
typedef void   typeVDPAUMapSurfacesNV (GLsizei numSurfaces,const GLvdpauSurfaceNV *surfaces);
typedef void   typeVDPAUUnmapSurfacesNV (GLsizei numSurface, const GLvdpauSurfaceNV *surfaces);
typedef void   typeVDPAUSurfaceAccessNV (GLvdpauSurfaceNV surface,   int           access);
typedef void   typeVDPAUFiniNV(void);
/***/
typeVDPAUInitNV                     *VDPAUInitNV=NULL;
typeVDPAURegisterOutputSurfaceNV    *VDPAURegisterOutputSurfaceNV=NULL;
typeVDPAUUnregisterSurfaceNV        *VDPAUUnregisterSurfaceNV=NULL;
typeVDPAUMapSurfacesNV              *VDPAUMapSurfacesNV=NULL;
typeVDPAUUnmapSurfacesNV            *VDPAUUnmapSurfacesNV=NULL;
typeVDPAUSurfaceAccessNV            *VDPAUSurfaceAccessNV=NULL;
typeVDPAUFiniNV                     *VDPAUFiniNV=NULL;
/***/
#define GETFUNC(x)   x = (type##x *)ADM_getGlWidget()->context()->getProcAddress(QLatin1String("gl"#x));\
    if(!x) \
    {\
            ADM_error("Cannot get "#x"\n");\
            exit(-1);\
    }
/**
    \fn processError
*/
static void processError(const char *e)
{
    int x=glGetError();
    if(x!=GL_NO_ERROR)
    {
        ADM_error("%s: Error : %d %s\n",e,x,gluErrorString(x));
    }
}
/**
    \fn initGl
*/
bool vdpauVideoFilterDeint::initOnceGl(void)
{
    ADM_info("Initializing VDPAU<->openGl\n");
    if(nbNvInit)
    {
        ADM_info("Already done..\n");
        nbNvInit++;
        return true;
    }
    GETFUNC(VDPAUInitNV);
    GETFUNC(VDPAURegisterOutputSurfaceNV);
    GETFUNC(VDPAUUnregisterSurfaceNV);  
    GETFUNC(VDPAUMapSurfacesNV);  
    GETFUNC(VDPAUUnmapSurfacesNV);  
    GETFUNC(VDPAUSurfaceAccessNV);  
    GETFUNC(VDPAUFiniNV);
    VDPAUInitNV(admVdpau::getVdpDevice(),admVdpau::getProcAddress());
    processError("InitNv");
    nbNvInit++;
    return true;
}
/**
     \fn initGl
*/
bool vdpauVideoFilterDeint::initGl(void)
{
   initOnceGl();
   rgb=new glRGB(this,NULL);
   return true;
}

/**
    \fn deInitGl
*/
bool vdpauVideoFilterDeint::deInitGl(void)
{
    ADM_info("De-Initializing VDPAU<->openGl\n");
    delete rgb;
    rgb=NULL;
    if(nbNvInit!=1)
    {
        ADM_info("still used..\n");
        if(nbNvInit) nbNvInit--;
        return true;
    }
    VDPAUFiniNV();
    nbNvInit--;
    return true;
}

/**
    \fn     getResult
    \brief  Convert the output surface into an ADMImage
*/
bool vdpauVideoFilterDeint::getResult(ADMImage *image)
{
    return rgb->surfaceToImage(outputSurface,image);
#ifdef DO_BENCHMARK
    ADMBenchmark bmark;
    for(int i=0;i<NB_BENCH;i++)
    {
        bmark.start();
#endif
  
    if(VDP_STATUS_OK!=admVdpau::outputSurfaceGetBitsNative(outputSurface,
                                                            tempBuffer, 
                                                            info.width,info.height))
    {
        ADM_warning("[Vdpau] Cannot copy back data from output surface\n");
        return false;
    }
  
                     
#ifdef DO_BENCHMARK
        bmark.end();
    }
    ADM_warning("Read surface Benchmark\n");
    bmark.printResult();
#endif 
    // Convert from VDP_RGBA_FORMAT_B8G8R8A8 to YV12
    uint32_t sourceStride[3]={info.width*4,0,0};
    uint8_t  *sourceData[3]={tempBuffer,NULL,NULL};
    uint32_t destStride[3];
    uint8_t  *destData[3];

    image->GetPitches(destStride);
    image->GetWritePlanes(destData);

    // Invert U&V
    uint32_t ts;
    uint8_t  *td;
#if 0
    ts=destStride[2];destStride[2]=destStride[1];destStride[1]=ts;
    td=destData[1];destData[2]=destData[2];destData[1]=td;
#endif


#ifdef DO_BENCHMARK
    ADMBenchmark bmark2;
    for(int i=0;i<NB_BENCH;i++)
    {
        bmark2.start();
#endif
    scaler->convertPlanes(  sourceStride,destStride,     
                            sourceData,destData);
#ifdef DO_BENCHMARK
        bmark2.end();
    }
    ADM_warning("RGB->YUV Benchmark\n");
    bmark2.printResult();
#endif

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
        printf("Compiling shader \n");
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
    \fn surfaceToImage
*/
bool   glRGB::surfaceToImage(VdpOutputSurface surf,ADMImage *image)
{
    widget->makeCurrent();
    glPushMatrix();
    // size is the last one...
    fboY->bind();
    processError("Bind");
    glProgramY->setUniformValue("myTextureY", 0); 
    glProgramY->setUniformValue("myWidth", image->GetWidth(PLANAR_Y)); 
    glProgramY->setUniformValue("myHeight", image->GetHeight(PLANAR_Y)); 

    myGlActiveTexture(GL_TEXTURE0);
    processError("Active Texture");
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, texName[0]); 
    processError("Bind Texture");
    //---
    GLvdpauSurfaceNV s=VDPAURegisterOutputSurfaceNV((const void *)surf,
                                                        GL_TEXTURE_2D,1,texName);
    printf("S=%x\n",(int)s);
    processError("Register");
    VDPAUSurfaceAccessNV(s,GL_READ_ONLY);
    VDPAUMapSurfacesNV(1,&s);
    processError("Map");

    render(image,PLANAR_Y,fboY);
    downloadTextures(image,fboY);

    VDPAUUnmapSurfacesNV(1,&s);
    processError("Unmap");
    VDPAUUnregisterSurfaceNV(s);
    processError("Unregister");
    fboY->release();
    firstRun=false;
    glPopMatrix();
    widget->doneCurrent();
  

    return true;
}


//****************
// EOF
