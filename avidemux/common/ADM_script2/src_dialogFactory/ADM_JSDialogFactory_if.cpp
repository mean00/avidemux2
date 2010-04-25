/**
    \file   ADM_JSDialogFactory_if.cpp
    \brief  JS / DF binding
    \author gruntster, mean (c) 2010


    jsapigen does not like much variable number of arguments
    In that case, we patch the generated file to go back to native spidermonkey api


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "jsapi.h"
#include "DIA_factory.h"
#include "ADM_JSDFMenu.h"
#include "ADM_JSDFToggle.h"
#include "ADM_JSDialogFactory.h"

/**
    \fn ADM_JSDialogFactoryInit()
    \brief Hook classes to spidermonkey
*/
bool ADM_JSDialogFactoryInit(JSContext *cx, JSObject *obj)
{
    if(NULL==ADM_JSDialogFactory::JSInit(cx,obj))
    {
        ADM_error("Cannot register dialogFactory js class\n");
        return false;
    }
    if(NULL==ADM_JSDFMenu::JSInit(cx,obj))
    {
        ADM_error("Cannot register  menu js class\n");
        return false;
    }
    if(NULL==ADM_JSDFToggle::JSInit(cx,obj))
    {
        ADM_error("Cannot register toggle js class\n");
        return false;
    }
    ADM_info("Registered DialogFactory classes\n");
    return true;
}