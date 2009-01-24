//
//
//
#ifndef MPEG2ENC_PARM
#define MPEG2ENC_PARM

typedef struct Mpeg2encParam
{	
	uint32_t	maxBitrate;
	uint32_t        gop_size;	
	uint32_t        widescreen;
	uint32_t        user_matrix;
	uint32_t        interlaced;
	uint32_t        bff;

}Mpeg2encParam;


#endif
