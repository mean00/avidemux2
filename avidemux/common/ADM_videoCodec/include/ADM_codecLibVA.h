/**
        \file FFLIBVA
 *      \brief wrapper around ffmpeg wrapper around libva
 */
#ifdef USE_LIBVA
#include "X11/Xlib.h"
#include "va/va.h"

#include <BVector.h>
#include "ADM_coreVideoCodec/ADM_hwAccel/ADM_coreLibVA/include/ADM_coreLibVA.h"

/**
 * \class decoderFFLIBVA
 */
#define ADM_MAX_SURFACE 17


class decoderFFLIBVA:public decoderFF
{
protected:
protected:
                    bool          alive;
                    int           b_age;
                    int           ip_age[2];
                    ADMImage      *scratch;
                    uint8_t       *yv12Buffer;
                    bool          decode_status;
                    VAContextID   libva;
                    int           nbSurface;
                    vaapi_context *va_context;
                    VASurfaceID   surfaces[ADM_MAX_SURFACE];
                    VAImage       *intermediateImage;
                    BVector <VASurfaceID  > freeQueue;
public:     // Callbacks
                    int         getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void        releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);
                    void        goOn(  AVFrame *d,int type);   
                    
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
};
#endif
