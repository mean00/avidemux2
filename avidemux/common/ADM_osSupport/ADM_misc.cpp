#include <math.h>

#include "ADM_default.h"

char* ms2timedisplay(uint32_t ms)
{
	uint32_t mm, ss;
#define ADM_MAX_STRING 256
	static char string[ADM_MAX_STRING+1];

	mm = (uint32_t)floor(ms / 60000.);
	
	if (mm > 1)
	{
		snprintf(string, ADM_MAX_STRING,QT_TRANSLATE_NOOP("adm","%" PRIu32" minutes"), mm);
	}
	else if (mm == 1)
	{
		snprintf(string,ADM_MAX_STRING, QT_TRANSLATE_NOOP("adm","%" PRIu32" minute"), mm);
	}
	else
	{
		ss = (uint32_t)floor(ms / 1000.);

		if (ss == 1)
		{
			snprintf(string,ADM_MAX_STRING, QT_TRANSLATE_NOOP("adm","%" PRIu32" second"), ss);
		}
		else
		{
			snprintf(string,ADM_MAX_STRING, QT_TRANSLATE_NOOP("adm","%" PRIu32" seconds"), ss);
		}
	}
    string[ADM_MAX_STRING]=0;
	return string;
}

