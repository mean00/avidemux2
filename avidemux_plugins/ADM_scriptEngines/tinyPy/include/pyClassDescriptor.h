#ifndef PYCLASSDESCRIPTOR_H
#define PYCLASSDESCRIPTOR_H

#include <string>

typedef struct
{
	std::string className;
	std::string desc;
        bool staticClass;
} pyClassDescriptor;

#endif
