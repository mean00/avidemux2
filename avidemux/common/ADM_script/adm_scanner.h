/*
	Type used by scanner

*/
#ifndef ADMSCANNER
#define ADMSCANNER

#define ADM_MAX_VAR 50
//#include "ADM_videoFilter_iface.h"

typedef enum
{
	ASC_OK,
	ASC_UNKNOWN_FUNC,
	ASC_BAD_NUM_PARAM,
	ASC_BAD_PARAM,
	ASC_EXEC_FAILED
}ASC_ERROR;

//int PushParam(APM_TYPE, char *value);
int Call(char *string);


#endif
//END

