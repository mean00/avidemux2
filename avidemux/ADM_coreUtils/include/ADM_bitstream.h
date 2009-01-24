
#ifndef ADM_BITSTREAM_H
#define ADM_BITSTREAM_H
#define ADM_NO_TIMING 0xffffffff

/*
    BIG WARNING : BUFFER SIZE MUST BE SET: SOME CODECS CHECK& USE IT
    ESPECIALLY LAVCODEC!
*/

class ADMBitstream
{
    public:
        uint32_t len;
        uint32_t bufferSize;
        uint8_t *data;
        uint32_t flags;
        uint32_t in_quantizer;          // Quantizer asked
        uint32_t out_quantizer;         // Quantizer of the image, in case of encoding the real Q
        uint32_t ptsFrame;              // Frame number in display order
        uint32_t dtsFrame;              // Frame number in container order (decoding order)
        uint64_t pts;			// in ms
        uint64_t dts;			// in ms

        ADMBitstream (uint32_t buffersize=0);
        ~ADMBitstream ();
        void cleanup (uint32_t dts);

};
#endif
