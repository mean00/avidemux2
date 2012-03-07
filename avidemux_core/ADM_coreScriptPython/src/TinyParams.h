#ifndef TINYPARAMS_H
#define TINYPARAMS_H

#ifndef CPYTHON_MOD
#define CPYTHON_MOD
#endif

#include "tinypy.h"
#include "ADM_inttype.h"
#include "ADM_confCouple.h"

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
