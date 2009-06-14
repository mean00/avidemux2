//
// C++ Implementation: ADM_vidForcedPP
//
// Description: 
//
//	Force postprocessing assuming constant quant & image type
//	Uselefull on some badly authored DVD for example
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"
#include "ADM_default.h"
#include "DIA_factory.h"

/**
      \fn DIA_lavDecoder
      \brief Dialog for lavcodec *DECODER* option
*/
uint8_t DIA_lavDecoder(uint32_t *swapUv, uint32_t *showU)
{
int ret=0;	
         
         diaElemToggle    swap(swapUv,QT_TR_NOOP("_Swap U and V"));
         diaElemToggle    show(showU,QT_TR_NOOP("Show motion _vectors"));
         diaElem *tabs[]={&swap,&show};
        if( diaFactoryRun(QT_TR_NOOP("Decoder Options"),2,tabs))
	{
          return 1;
        }
         return 0;
}
// EOF
