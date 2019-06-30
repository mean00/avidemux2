/**
        \file FFDXVA2
 *      \brief wrapper around ffmpeg wrapper around DXVA2
 */


#include "ADM_threads.h"
#include "ADM_edCache.h"
#include "ADM_filterThread.h"

#define ADM_MAX_SURFACE (EDITOR_CACHE_MAX_SIZE + ADM_THREAD_QUEUE_SIZE + 16)
/**
      \struct surface_info
*/
typedef struct surface_info
{
    int used;
    uint64_t age;
} surface_info;
/**
 * \class admDx2Surface
 */
class decoderFFDXVA2;

typedef struct
{
        BVector <admDx2Surface *>freeSurfaceQueue;
        BVector <admDx2Surface *>allSurfaceQueue;
}dxvaContext;

/**
 * \class decoderFFDXVA2
 */
class decoderFFDXVA2:public ADM_acceleratedDecoderFF
{
protected:
                    bool                        alive;
                    int                         align;;
                    LPDIRECT3DSURFACE9          surfaces[ADM_MAX_SURFACE];
                    surface_info                surface_infos[ADM_MAX_SURFACE];
                    uint32_t                    num_surfaces;
                    uint64_t                    surface_age;
                    dxvaContext                 dxvaPool;
protected:
                    bool        initDXVA2Context();
public:
//                    bool        markSurfaceUsed(ADM_vaSurface *s);
                    //bool        markSurfaceUnused(ADM_vaSurface *s);
public:     // Callbacks
                    //int         getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    //void        releaseBuffer(ADM_vaSurface *vaSurface);
                    bool        initFail(void) {alive=false;return true;}
public:
    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);
                    int         getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    bool        releaseBuffer(admDx2Surface *data);
                    admDx2Surface        *findBuffer(LPDIRECT3DSURFACE9 surface);
                    bool        isAlive() {return alive;}
                    int         getNumSurfaces() {return num_surfaces;}

    virtual const   char        *getName(void)        {return "DXVA2";}
    virtual         bool        dontcopy() {return true;} // copy the frame for the moment

public:
            // public API
                                decoderFFDXVA2 (AVCodecContext *avctx,decoderFF *parent);
                                ~decoderFFDXVA2();
public:
                    bool      markSurfaceUsed(admDx2Surface *s);
                    bool      markSurfaceUnused(admDx2Surface *s);
protected:
                    bool      readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out);


};
