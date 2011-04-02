#include "JSONDebug.h"
#ifdef JSON_DEBUG

#ifdef JSON_STDERROR
    #include <iostream>  //need std::cerr
#else
    //otherwise, use a callback to tell the end user what happened
    json_error_callback_t ErrorCallback = 0;
    void JSONDebug::register_callback(json_error_callback_t callback) json_nothrow {
	   ErrorCallback = callback;
    }
#endif

//Something went wrong or an assert failed
void JSONDebug::_JSON_FAIL(const json_string & msg) json_nothrow {
    #ifdef JSON_STDERROR  //no callback, just use stderror
	   #ifndef JSON_UNICODE
		  std::cerr << msg << std::endl;
	   #else
		  std::cerr << std::string(msg.begin(), msg.end()) << std::endl;
	   #endif
    #else
	   if (json_likely(ErrorCallback != 0)){  //only do anything if the callback is registered
		  #ifdef JSON_LIBRARY
			 ErrorCallback(msg.c_str());
		  #else
			 ErrorCallback(msg);
		  #endif
	   }
    #endif
}

//asserts that condition is true, more useful than cassert because it lets you keep going
void JSONDebug::_JSON_ASSERT(bool condition, const json_string & msg) json_nothrow {
    if (json_unlikely(!condition)){
	   _JSON_FAIL(msg);
    }
}
#endif
