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

#include "TinyParams.h"

#define preamble(xtype) tp_obj obj = TP_OBJ();\
						if (obj.type != xtype) \
                        { \
                            raise("Expected %s, got %s\n", typeAsString(xtype), typeAsString(obj.type)); \
                        }
/**
   \fn  asInt
*/
int TinyParams::asInt(void)
{
	preamble(TP_NUMBER);
	return (int)obj.number.val;
}
/**
   \fn  asDouble
*/

double TinyParams::asDouble(void)
{
	preamble(TP_NUMBER);
	return (double)obj.number.val;
}
/**
   \fn  asString
*/

const char *TinyParams::asString(void)
{
	preamble(TP_STRING);
	return obj.string.val;
}
/**
   \fn  asThis
*/

void *TinyParams::asThis(tp_obj *self, int id)
{
	tp_obj cdata = tp_get(tp, *self, tp_string("cdata"));

	if (cdata.data.magic != id)
	{
		raise("Bad class : Expected %d, got %d\n", id, cdata.data.magic);
		\
	}

	return cdata.data.val;
}
/**
   \fn  asObjectPointer
*/
void *TinyParams::asObjectPointer(void)
{
	preamble(TP_DICT);
	tp_obj cdata = tp_get(tp, obj, tp_string("cdata"));
	return cdata.data.val;

}
/**
   \fn  typeAsString
    \brief return the type given as a string
*/

const char *TinyParams::typeAsString(int type)
{
	switch (type)
	{
		case TP_NUMBER:
			return "Number";
			break;

		case TP_STRING:
			return "String";
			break;

		case TP_LIST:
			return "List";
			break;

		case TP_DICT:
			return "Dict";
			break;

		case TP_FNC:
			return "Function";
			break;

		case TP_DATA:
			return "Data";
			break;
	}

	return "???";
}
/**
    \fn raise
    \brief raise an exception
*/
void TinyParams::raise(const char *fmt, ...)
{
	char print_buffer[1024];
	va_list         list;
	va_start(list,  fmt);
	vsnprintf(print_buffer, 1023, fmt, list);
	va_end(list);
	print_buffer[1023] = 0; // ensure the string is terminated
	_tp_raise(tp, tp_None);
}
/**
    \fn makeCouples
    \brief convert couples into char *first and *couples c, c can be null
*/
bool    TinyParams::makeCouples(CONFcouple **c)
{
	int nb = nbParamsLeft();

	if (!nb)
	{
		*c = NULL;
		return true;
	}

	const char *s[nb];

	for (int i = 0; i < nb; i++)
	{
		s[i] = asString();
	}

	return stringsToConfCouple(nb, c, s);
}
