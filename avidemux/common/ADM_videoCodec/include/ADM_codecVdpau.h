/**
 *  \class decoderFFVDPAU
 */
#ifdef USE_VDPAU
struct AVVDPAUContext;
class decoderFFVDPAU:public decoderFF
{
protected:
                    bool    alive;
                    void     *vdpau;
                    bool     decode_status;   
                    AVVDPAUContext *avVdCtx;
protected:
                    bool        initVdpContext();
public:     // Callbacks
                    int         getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void        releaseBuffer(struct vdpau_render_state *rdr);
public:
            // public API
                                decoderFFVDPAU (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
                                ~decoderFFVDPAU();
    virtual         bool        uncompress (ADMCompressedImage * in, ADMImage * out);
                    bool        readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out);    

    virtual         bool        dontcopy (void)             {     return false; } // We cannot use ffmpeg internal buffer, they dont exist!
    virtual         bool        bFramePossible (void)       {        return true;      }
    virtual const   char        *getDecoderName(void)        {return "VDPAU";}
    virtual         bool        initializedOk(void)         {return alive;};
                    
};
#endif
