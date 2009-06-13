#ifndef ADM_DCAINFO_H
#define ADM_DCAINFO_H
int ADM_DCAGetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *br, uint32_t *chan,uint32_t *syncoff,uint32_t *flags,uint32_t *nbSample);

#define DTS_HEADER_SIZE (10)
#endif


