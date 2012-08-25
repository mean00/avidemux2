/** **************************************************************************
        \fn DIA_flyDialogQt4.h
 copyright            : (C) 2007 by mean
 email                : fixounet@free.fr
 
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_FLY_DIALOG_CLI_H
#define ADM_FLY_DIALOG_CLI_H

#include "ADM_UI_Cli6_export.h"
#include "DIA_flyDialog.h"

class ADM_UI_CLI6_EXPORT ADM_flyDialogCli : public ADM_flyDialog
{
public:
  ADM_flyDialogCli(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              void *canvas, void *slider, int yuv, ResizeMethod resizeMethod):
                                ADM_flyDialog(width,height,in,canvas,slider,yuv,resizeMethod) {};
  virtual           ~ADM_flyDialogCli(void);
  virtual bool     isRgbInverted(void);
  virtual uint8_t  display(void);
  virtual float   calcZoomFactor(void);
  virtual uint32_t sliderGet(void);
  virtual uint8_t  sliderSet(uint32_t value);
  virtual void    postInit(uint8_t reInit);
  virtual void    setupMenus (const void * params,
                         const MenuMapping * menu_mapping,
                        uint32_t menu_mapping_count) ;
  virtual void  getMenuValues ( void * mm,
                    const MenuMapping * menu_mapping,
                    uint32_t menu_mapping_count) ;
  virtual  const MenuMapping *lookupMenu (const char * widgetName,
                                               const MenuMapping * menu_mapping,
                                               uint32_t menu_mapping_count) ;
  virtual uint32_t  getMenuValue (const MenuMapping * mm) ;          
};


#endif
