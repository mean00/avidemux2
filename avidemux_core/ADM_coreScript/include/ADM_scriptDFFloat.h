/**
    \file   ADM_JSDFFloat.h
    \brief  JS / DF binding
    \author gruntster/mean 2010


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADM_scriptDFFloat_H
#define _ADM_scriptDFFloat_H

#include "ADM_coreScript_export.h"
#include "DIA_factory.h"
#include "ADM_scriptDF.h"
/**
    \class ADM_JSDFFloatHelper
*/
class ADM_CORESCRIPT_EXPORT ADM_scriptDFFloatHelper : public ADM_scriptDFBaseHelper
{
private:
	char                *_title;
	double             _value;
        double             _min,_max;
        int                _precision;

public:
                     ADM_scriptDFFloatHelper(const char *title,double mn, double mx, int prec);
                     ~ADM_scriptDFFloatHelper(void);
	virtual diaElem* getControl(void);
            double   value(void);
            void     setValue(double value);
};
#endif
