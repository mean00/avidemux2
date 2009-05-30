#include <math.h>

#include "ADM_default.h"

char* ms2timedisplay(uint32_t ms)
{
	uint32_t mm, ss;
	static char string[20];

	mm = (uint32_t)floor(ms / 60000.);
	
	if (mm > 1)
	{
		sprintf(string, QT_TR_NOOP("%lu minutes"), mm);
	}
	else if (mm == 1)
	{
		sprintf(string, QT_TR_NOOP("%lu minute"), mm);
	}
	else
	{
		ss = (uint32_t)floor(ms / 1000.);

		if (ss == 1)
		{
			sprintf(string, QT_TR_NOOP("%lu second"), ss);
		}
		else
		{
			sprintf(string, QT_TR_NOOP("%lu seconds"), ss);
		}
	}

	return string;
}

