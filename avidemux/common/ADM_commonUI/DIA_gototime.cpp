#include "config.h"
#include "ADM_default.h"
#include "DIA_factory.h"

uint8_t DIA_gotoTime(uint16_t *hh, uint16_t *mm, uint16_t *ss)
{
uint32_t h=*hh,m=*mm,s=*ss;

diaElemUInteger   eh(&h,QT_TR_NOOP("_Hours:"),0,24);
diaElemUInteger   em(&m,QT_TR_NOOP("_Minutes:"),0,59);
diaElemUInteger   es(&s,QT_TR_NOOP("_Seconds:"),0,59);
        diaElem *allWidgets[]={&eh,&em,&es};

  if(!diaFactoryRun(QT_TR_NOOP("Go to Time"),3,allWidgets)) return 0;
    *hh=h;
    *mm=m;
    *ss=s;
    return 1;

}
