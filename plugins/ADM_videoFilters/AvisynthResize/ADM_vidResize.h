/**
    \file ADM_vidResize.h


*/
#ifndef ADM_VIDRESIZE_H
#define ADM_VIDRESIZE_H
class  AVDMVideoStreamResize:public AVDMGenericVideoStream
 {

 protected:
            RESIZE_PARAMS           *_param;
            uint8_t             _init;
            INT                     *Hpattern_luma;
            INT                     *Hpattern_chroma;
            INT                     *Vpattern_luma;
            INT                     *Vpattern_chroma;
            uint8_t                 *_intermediate_buffer;
        // engine
        void ResizeV(Image * iin, Image * iout, INT *pattern) ;
        void ResizeVFIR4(Image * iin, Image * iout, INT *pattern) ;

        void ResizeH(Image * iin, Image * iout, INT *pattern) ;
        void ResizeHFIR4(Image * iin, Image * iout, INT  *pattern)  ;

        void precompute(Image * dst, Image * src, uint8_t algo);
            uint8_t     zoom(Image * dst, Image * src);
            void endcompute(void);

 public:

                AVDMVideoStreamResize(  AVDMGenericVideoStream *in,CONFcouple *setup);
                AVDMVideoStreamResize(  AVDMGenericVideoStream *in,uint32_t x,uint32_t y);
                virtual         ~AVDMVideoStreamResize();
          virtual       uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                                            ADMImage *data,uint32_t *flags);
                uint8_t configure( AVDMGenericVideoStream *instream);
    virtual         char    *printConf(void) ;

          virtual uint8_t   getCoupledConf( CONFcouple **couples);


 }     ;
 #endif
