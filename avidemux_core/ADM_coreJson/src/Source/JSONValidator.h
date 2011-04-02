#ifndef JSON_VALIDATOR_H
#define JSON_VALIDATOR_H

#include "JSONDebug.h"

#ifdef JSON_VALIDATE

class JSONValidator {
    public:
	   static bool isValidNumber(const json_char * & ptr) json_nothrow json_read_priority;
	   static bool isValidMember(const json_char * & ptr) json_nothrow json_read_priority;
	   static bool isValidString(const json_char * & ptr) json_nothrow json_read_priority;
	   static bool isValidNamedObject(const json_char * & ptr) json_nothrow json_read_priority;
	   static bool isValidObject(const json_char * & ptr) json_nothrow json_read_priority;
	   static bool isValidArray(const json_char * & ptr) json_nothrow json_read_priority;
	   static bool isValidRoot(const json_char * json) json_nothrow json_read_priority;
};

#endif

#endif
