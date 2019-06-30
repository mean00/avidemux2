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

#include "ADM_default.h"
#include "DIA_factory.h"
/**
      \fn DIA_lavDecoder
      \brief Dialog for lavcodec *DECODER* option
*/
uint8_t DIA_lavDecoder(bool *swapUv)
{
         diaElemToggle    swap(swapUv,QT_TRANSLATE_NOOP("lavdecoder","_Swap U and V"));
         diaElem *tabs[]={&swap};
        if( diaFactoryRun(QT_TRANSLATE_NOOP("lavdecoder","Decoder Options"),1,tabs))
	{
          return 1;
        }
         return 0;
}
// EOF
