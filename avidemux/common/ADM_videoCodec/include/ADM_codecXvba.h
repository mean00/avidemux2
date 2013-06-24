/**
        \file FFXVBA
 *      \brief wrapper around ffmpeg wrapper around xvba
 */
#ifdef USE_XVBA
#include "X11/Xlib.h"
#include "amd/amdxvba.h"

#include <ADM_ptrQueue.h>
struct xvba_render_state;
/**
 * \class decoderFFXVBA
 */
class decoderFFXVBA:public decoderFF
{
protected:
                    XVBABufferDescriptor  *pictureDescriptor;
                    XVBABufferDescriptor  *dataBuffer;
                    XVBABufferDescriptor  *qmBuffer;
                    XVBABufferDescriptor  *ctrlBuffer[50];
                    int   ctrlBufferCount;;
                    uint8_t *tmpYV12Buffer;
protected:
                    bool alive;
                    int b_age;
                    int ip_age[2];
                    ADM_ptrQueue <xvba_render_state> freeQueue;
                    ADM_ptrQueue <xvba_render_state> inUseQueue;
                    void     *xvba;
                    ADMImage *scratch;
                    ADMImage *xvba_copy;
                    uint64_t xvba_pts;
                    bool     decode_status;
                    bool     destroying;
public:     // Callbacks
                    int     getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void    releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);
                    void    goOn( const AVFrame *d,int type);            
public:
            // public API
                    decoderFFXVBA (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
                    ~decoderFFXVBA();
    virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);

    virtual bool dontcopy (void)
                      {
                        return 0; // We cannot use ffmpeg internal buffer, they dont exist!
                      }

    virtual bool bFramePossible (void)
      {
        return 1;
      }
    virtual const char *getDecoderName(void) {return "XVBA";}
    virtual bool  initializedOk(void)  {return alive;};
};
#endif
