/*
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADM_coreConfig.h"
#include "ADM_default.h"
#include "ADM_memsupport.h"


adm_fast_memcpy myAdmMemcpy=NULL;
/*

	It seems MMX gives the best result most of the times
	Don't bother benchmarking
*/
uint8_t ADM_InitMemcpy(void)
{
#undef memcpy
        myAdmMemcpy=memcpy;
	return 1;
}
