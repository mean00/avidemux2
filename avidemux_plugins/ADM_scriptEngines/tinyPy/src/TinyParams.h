/***************************************************************************
   \file ADM_pyAvidemux.cpp
    \brief binding between tinyPy and avidemux
    \author mean/gruntster 2011/2012
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TINYPARAMS_H
#define TINYPARAMS_H

#ifndef CPYTHON_MOD
#define CPYTHON_MOD
#endif

#include "tinypy.h"
#include "ADM_inttype.h"
#include "ADM_confCouple.h"
/**
    \class TinyParams
*/
class TinyParams
{
protected:
	tp_vm *tp;

	int nbParamsLeft(void)
	{
		return tp->params.list.val->len;
	}

public:
	TinyParams(tp_vm *i)
	{
		tp = i;
	}

	int    asInt(void);
	double asDouble(void);
	const   char  *asString(void);
	void  *asThis(tp_obj *self, int id);
	void  *asObjectPointer(void);
	int    nbParam(void);
	void   raise(const char *fmt, ...);
	const char *typeAsString(int type);
	bool   makeCouples(CONFcouple **c);
};

#endif
