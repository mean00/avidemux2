

#ifndef ADM_MPLAYER_H
#define ADM_MPLAYER_H

class ADM_MplayerResize
{
protected:
                void *_context;
                uint32_t srcW,srcH;
                uint32_t tgtW,tgtH;
        
public:
                ADM_MplayerResize(uint32_t fromw, uint32_t fromh, uint32_t tow, uint32_t toh);
                ~ADM_MplayerResize();
                uint8_t resize(uint8_t *src, uint8_t *dst);


};

#endif


