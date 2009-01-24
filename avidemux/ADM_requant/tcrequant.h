extern int Mrequant_init (float quant_factor,int byteStuffing);
extern int Mrequant_frame(uint8_t *in, uint32_t len,uint8_t *out, uint32_t *lenout);
extern int Mrequant_end (void);
