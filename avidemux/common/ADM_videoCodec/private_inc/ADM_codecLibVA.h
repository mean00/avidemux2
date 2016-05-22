/**
        \file FFLIBVA
 *      \brief wrapper around ffmpeg wrapper around libva
 */

#include "X11/Xlib.h"
#include "va/va.h"

#include <BVector.h>
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreLibVA/include/ADM_coreLibVA.h"
#include "ADM_threads.h"
/**
 * \class decoderFFLIBVA
 */
#define ADM_MAX_SURFACE 16+6+1

typedef struct 
{
        BVector <ADM_vaSurface *>freeSurfaceQueue;
        BVector <ADM_vaSurface *>allSurfaceQueue;
}libvaContext;


class decoderFFLIBVA:public ADM_acceleratedDecoderFF
{
friend class ADM_vaSurface;
protected:
protected:
                    bool          alive;
                    libvaContext  vaPool;

protected:
                    bool        initVAContext();
public:                    
                    bool        markSurfaceUsed(ADM_vaSurface *s);
                    bool        markSurfaceUnused(ADM_vaSurface *s);
                    bool        markSurfaceUnused(VASurfaceID id);
public:     // Callbacks
                    int         getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void        releaseBuffer(ADM_vaSurface *vaSurface);
                    bool        initFail(void) {alive=false;return true;}
public:
    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);
                    bool        readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out);    
    virtual const   char        *getName(void)        {return "LIBVA";}
                  ADM_vaSurface *lookupBySurfaceId(VASurfaceID id);
                    
public:
            // public API
                                decoderFFLIBVA (AVCodecContext *avctx,decoderFF *parent);
                                ~decoderFFLIBVA();
};
