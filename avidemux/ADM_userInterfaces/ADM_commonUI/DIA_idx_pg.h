#ifndef DIA_IDX_PG
#define DIA_IDX_PG


class DIA_progressIndexing
{
protected:
                Clock           clock;
                uint32_t        aborted;
		uint32_t	_nextUpdate;
        
public:
                        DIA_progressIndexing(const char *name);
                        ~DIA_progressIndexing();
          uint8_t       update(uint32_t done,uint32_t total, uint32_t nbImage, uint32_t hh, uint32_t mm, uint32_t ss);
          uint8_t        abortRequest(void);
          uint8_t       isAborted(void) ;

};

#endif
