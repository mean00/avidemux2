/**
        \file FFDXVA2
 *      \brief wrapper around ffmpeg wrapper around DXVA2
 */


#include <BVector.h>
#include "ADM_coreDXVA2.h"
#include "ADM_threads.h"
/**
 * \class decoderFFDXVA2
 */
class decoderFFDXVA2:public ADM_acceleratedDecoderFF
{
protected:
protected:
                    bool        alive;

protected:
                    bool        initDXVA2Context();
public:                    
//                    bool        markSurfaceUsed(ADM_vaSurface *s);
                    //bool        markSurfaceUnused(ADM_vaSurface *s);
public:     // Callbacks
                    int         getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void        releaseBuffer(ADM_vaSurface *vaSurface);
                    bool        initFail(void) {alive=false;return true;}
public:
    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);
                    bool        readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out);    
    virtual const   char        *getName(void)        {return "DXVA2";}
                    
public:
            // public API
                                decoderFFDXVA2 (AVCodecContext *avctx,decoderFF *parent);
                                ~decoderFFDXVA2();
};
