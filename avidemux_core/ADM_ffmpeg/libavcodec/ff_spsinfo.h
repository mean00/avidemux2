#ifndef FF_SPS_INFO_H
#define FF_SPS_INFO_H

typedef struct
{
	int width;
	int height;
	int fps1000;
	int darNum;
	int darDen;
	int hasStructInfo;
	int CpbDpbToSkip;
}ffSpsInfo;


#endif

