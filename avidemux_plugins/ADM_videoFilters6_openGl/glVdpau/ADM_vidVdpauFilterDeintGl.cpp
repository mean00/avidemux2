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
/**
    \fn     getResult
    \brief  Convert the output surface into an ADMImage
*/
bool vdpauVideoFilterDeint::getResult(ADMImage *image)
{

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

//****************
// EOF
