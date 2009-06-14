#ifndef guiHelper_h
#define guiHelper_h

int x264_cqm_parse_file(const char *filename, uint8_t* intra4x4Luma, uint8_t* intra4x4Chroma, uint8_t* inter4x4Luma, 
						uint8_t* inter4x4Chroma, uint8_t* intra8x8Luma, uint8_t* inter8x8Luma);
#endif	// guiHelper_h
