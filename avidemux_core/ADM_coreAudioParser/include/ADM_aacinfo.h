//
//	Used to get info on a AAC/ADTS stream
//	Some parameters can be given to use as guideline
//

#ifndef ADM__AAC__INFO
#define ADM__AAC__INFO
/**
    \fn AacAudioInfo
*/
typedef struct 
{
	uint32_t frequency;		
	uint32_t channels;	
	bool     sbr;	
}AacAudioInfo;

/// extract fq etc.. from ESDS atom
bool getAdtsAacInfo(int size, uint8_t *data, AacAudioInfo &info);
#endif