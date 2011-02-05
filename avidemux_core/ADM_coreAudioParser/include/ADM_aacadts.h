

#ifndef ADM_AAC_ADTS
#define ADM_AAC_ADTS
/**
    \class ADM_adts2aac
*/
class ADM_adts2aac
{
private:
		void * cookie;
        void * codec;
		
public:
		bool getExtraData(uint32_t *len,uint8_t **data);
		bool convert(int incomingLen,uint8_t *intData,int *outLen,uint8_t *out);
        int getFrequency(void);
        int getChannels(void);
             ADM_adts2aac();
             ~ADM_adts2aac();

};
#endif

