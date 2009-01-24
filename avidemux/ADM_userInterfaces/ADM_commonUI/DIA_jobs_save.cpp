
#include "ADM_default.h"
#include "ADM_inttype.h"
#include "DIA_factory.h"

uint8_t DIA_job_select(char **jobname, char **filename)
{
	diaElemText jobNameWidget(jobname, QT_TR_NOOP("_Job name:"), NULL);
	diaElemFile outputFileWidget(1, filename, QT_TR_NOOP("Output _File:"), NULL, QT_TR_NOOP("Select Video To Write"));
	diaElem *elem[] = {&jobNameWidget, &outputFileWidget};

	return diaFactoryRun(QT_TR_NOOP("Save Job"), 2, elem);
}
