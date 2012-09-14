/***************************************************************************
  FAC_TimeStamp.cpp
  Handle dialog factory element: Progress TimeStamp
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

#include "ADM_toolkitGtk.h"
#include "DIA_factory.h"
namespace ADM_GtkFactory
{
class diaElemTimeStamp : public diaElem
{
  protected :
        uint32_t *per;
public:
  
  diaElemTimeStamp(uint32_t *percent,const char *toggleTitle,const uint32_t vmin, const uint32_t vmax);
  virtual ~diaElemTimeStamp() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};
/**
*/
diaElemTimeStamp::diaElemTimeStamp(uint32_t *percent,const char *toggleTitle,const uint32_t vmin, const uint32_t vmax)
  : diaElem(ELEM_TIMESTAMP)
{
  per=percent;
  paramTitle=ADM_strdup(toggleTitle);
}

diaElemTimeStamp::~diaElemTimeStamp()
{
  ADM_dealloc(paramTitle);
}
void diaElemTimeStamp::setMe(void *dialog, void *opaque,uint32_t line)
{
  
}
void diaElemTimeStamp::getMe(void)
{
  
}

int diaElemTimeStamp::getRequiredLayout(void) { return 0; }
} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateTimeStamp(uint32_t *percent,const char *toggleTitle,const uint32_t vmin, const uint32_t vmax)
{
	return new  ADM_GtkFactory::diaElemTimeStamp(percent,toggleTitle,vmin,vmax);
}
void gtkDestroyTimeStamp(diaElem *e)
{
	ADM_GtkFactory::diaElemTimeStamp *a=(ADM_GtkFactory::diaElemTimeStamp *)e;
	delete a;
}
//
//EOF
