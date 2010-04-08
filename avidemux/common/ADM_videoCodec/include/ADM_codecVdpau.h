#ifdef USE_VDPAU
class decoderFFVDPAU:public decoderFF
{
protected:
                    int b_age;
                    int ip_age[2];

                    void     *vdpau;
                    ADMImage *scratch;
                    ADMImage *vdpau_copy;
                    uint64_t vdpau_pts;
                    bool     decode_status;
                    bool     destroying;
public:     // Callbacks
                    int     getBuffer(AVCodecContext *avctx, AVFrame *pic);
                    void    releaseBuffer(struct AVCodecContext *avctx, AVFrame *pic);
                    void    goOn( const AVFrame *d,int type);            
public:
            // public API
                    decoderFFVDPAU (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
                    ~decoderFFVDPAU();
    virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);

    virtual bool dontcopy (void)
                      {
                        return 0; // We cannot use ffmpeg internal buffer, they dont exist!
                      }

    virtual bool bFramePossible (void)
      {
        return 1;
      }
};
#endif
