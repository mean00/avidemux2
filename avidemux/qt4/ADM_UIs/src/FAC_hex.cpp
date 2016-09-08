/***************************************************************************
  FAC_toggle.cpp
  Handle dialog factory element : Toggle
  (C) 2006 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

namespace ADM_qt4Factory
{
class diaElemHex : public diaElem,QtFactoryUtils
{
  uint32_t dataSize;
  uint8_t  *data;
  
public:
  
  diaElemHex(const char *toggleTitle, uint32_t dataSize,uint8_t *data);
  virtual ~diaElemHex() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) ;
  void finalize(void);
  int getRequiredLayout(void);
};
extern const char *shortkey(const char *);
  diaElemHex::diaElemHex(const char *toggleTitle, uint32_t dataSize,uint8_t *data) :diaElem(ELEM_HEXDUMP),QtFactoryUtils(toggleTitle)
  {
  };
  diaElemHex::~diaElemHex() {};
  void diaElemHex::setMe(void *dialog, void *opaque,uint32_t line) {};
  void diaElemHex::getMe(void) {} ;
  void diaElemHex::finalize(void) {};
  int diaElemHex::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }
//******************************************************
} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateHex(const char *toggleTitle, uint32_t dataSize,uint8_t *data)
{
	return new  ADM_qt4Factory::diaElemHex(toggleTitle,dataSize,data);
}
void qt4DestroyHex(diaElem *e)
{
	ADM_qt4Factory::diaElemHex *a=(ADM_qt4Factory::diaElemHex *)e;
	delete a;
}
//EOF


//EOF
