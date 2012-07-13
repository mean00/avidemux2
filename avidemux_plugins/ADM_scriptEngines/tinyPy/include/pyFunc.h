#ifndef PYFUNC_H
#define PYFUNC_H

typedef struct
{
	const char *funcName;
	tp_obj(*funcCall)(tp_vm *tp);
} pyFunc;

#endif