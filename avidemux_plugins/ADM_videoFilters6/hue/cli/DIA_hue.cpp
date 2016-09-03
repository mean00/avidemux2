#include "ADM_image.h"
#include "DIA_flyDialog.h"
#include "../hue.h"
#include "DIA_factory.h"
#include "../DIA_flyHue.h"
uint8_t DIA_getHue(hue *param, ADM_coreVideoFilter *in)
{
    
    diaElemFloat  hue(&(param->hue),QT_TRANSLATE_NOOP("hue","Hue :"),-180,180);
    diaElemFloat  sat(&(param->saturation),QT_TRANSLATE_NOOP("hue","Sat :"),-180,180);

    diaElem *elems[]={&hue,&sat};
    return diaFactoryRun("Hue",sizeof(elems)/sizeof(diaElem *),elems);
}
uint8_t    flyHue::download(void) {return 1;}
uint8_t    flyHue::upload(void) {return 1;}
