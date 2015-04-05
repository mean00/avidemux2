/**
        \file FFLIBVA
 *      \brief wrapper around ffmpeg wrapper around libva
 */
#ifdef USE_LIBVA
#include "X11/Xlib.h"
#include "va/va.h"

#include <BVector.h>
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreLibVA/include/ADM_coreLibVA.h"
#include "ADM_threads.h"
/**
 * \class decoderFFLIBVA
 */
#define ADM_MAX_SURFACE 16+6+1
class decoderFFLIBVA:public decoderFF
{
friend class ADM_vaSurface;
protected:
protected:
                    bool          alive;
                    int           b_age;
                    int           ip_age[2];
                    ADMImage      *scratch;
                    VAContextID   libva;
                    int           nbSurface;
                    vaapi_context *va_context;
                    VASurfaceID   surfaces[ADM_MAX_SURFACE];

                    //
                    BVector <ADM_vaSurface  *> freeSurfaceQueue;
                    BVector <ADM_vaSurface  *> allSurfaceQueue;
                    //
                    
                    
public:     // Callbacks
                    int         getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void        releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);
                    void        goOn(  AVFrame *d,int type);   
                    
                    bool        reclaimImage(ADM_vaSurface *img);
                    ADM_vaSurface *lookupBySurfaceId(VASurfaceID id);
                    
public:
            // public API
                                decoderFFLIBVA (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
                                ~decoderFFLIBVA();
    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);

    virtual         bool        dontcopy (void)
                      {
                        return 1; // For now, we give a reference to the fully decoded image
                      }

    virtual         bool        bFramePossible (void)
      {
        return 1;
      }
    virtual const   char        *getDecoderName(void) {return "LIBVA";}
    virtual         bool        initializedOk(void)  {return alive;};
static              bool        fccSupported(uint32_t fcc);
};
#endif
