
#include "ADM_default.h"
#include "ADM_inttype.h"
#include "DIA_factory.h"

uint8_t DIA_job_select(char **jobname, char **filename)
{
	diaElemText jobNameWidget(jobname, QT_TRANSLATE_NOOP("adm","_Job name:"), NULL);
        std::string file;
        
	diaElemFile outputFileWidget(1, file, QT_TRANSLATE_NOOP("adm","Output _File:"), NULL, QT_TRANSLATE_NOOP("adm","Select Video To Write"));
	diaElem *elem[] = {&jobNameWidget, &outputFileWidget};

	if(diaFactoryRun(QT_TRANSLATE_NOOP("adm","Save Job"), 2, elem))
        {
            *filename=ADM_strdup(file.c_str());
        }else
            *filename=NULL;
}
