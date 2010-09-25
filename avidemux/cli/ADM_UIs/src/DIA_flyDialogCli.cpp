/***************************************************************************
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************///



#include <math.h>
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyDialog.h"
#include "DIA_flyDialogCli.h"
#include "ADM_assert.h"

ADM_flyDialogCli::~ADM_flyDialogCli(void) { }

void ADM_flyDialogCli::postInit(uint8_t reInit)
{
}


float ADM_flyDialogCli::calcZoomFactor(void)
{
        return 0;      
}

uint8_t  ADM_flyDialogCli::display(void)
{
        return 0;      
}

uint32_t ADM_flyDialogCli::sliderGet(void)
{
        return 0;      
}

uint8_t ADM_flyDialogCli::sliderSet(uint32_t value)
{
        return 0;      
}

bool ADM_flyDialogCli::isRgbInverted(void)
{
        return 0; 
}

void ADM_flyDialogCli::setupMenus (const void * params, 
                                const MenuMapping * menu_mapping,
                                uint32_t menu_mapping_count)
{
        return ;
}

uint32_t ADM_flyDialogCli::getMenuValue (const MenuMapping * mm)
{
        return 0;
}

void ADM_flyDialogCli::getMenuValues ( void * params,
                                   const MenuMapping * menu_mapping,
                                   uint32_t menu_mapping_count)
{
        return ;
}

const MenuMapping *
ADM_flyDialogCli::lookupMenu (const char * widgetName,
                           const MenuMapping * menu_mapping,
                           uint32_t menu_mapping_count)
{
    return 0;
}

//EOF

