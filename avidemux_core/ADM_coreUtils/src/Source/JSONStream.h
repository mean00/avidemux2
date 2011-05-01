#ifndef JSONSTREAM_H
#define JSONSTREAM_H

#include "JSONDebug.h"

#ifdef JSON_STREAM

#ifdef JSON_MEMORY_CALLBACKS
    #include "JSONMemory.h"
#endif

#ifndef JSON_LIBRARY
    class JSONNode; //foreward declaration
    typedef void (*json_stream_callback_t)(JSONNode &);
#endif

class JSONStream {
public:
    JSONStream(json_stream_callback_t call_p) json_nothrow;
    JSONStream(const JSONStream & orig) json_nothrow;
    JSONStream & operator =(const JSONStream & orig) json_nothrow;
    #ifdef JSON_LIBRARY
	   JSONStream & operator << (const json_char * str) json_nothrow;
    #else
	   JSONStream & operator << (const json_string & str) json_nothrow;
    #endif

    static void deleteJSONStream(JSONStream * stream) json_nothrow {
	   #ifdef JSON_MEMORY_CALLBACKS
		  stream -> ~JSONStream();
		  libjson_free<JSONStream>(stream);
	   #else
		  delete stream;
	   #endif
    }

    static JSONStream * newJSONStream(json_stream_callback_t callback) json_nothrow {
	   #ifdef JSON_MEMORY_CALLBACKS
		  return new(json_malloc<JSONStream>(1)) JSONStream(callback);
	   #else
		  return new JSONStream(callback);
	   #endif
    }
JSON_PRIVATE
    size_t FindNextRelevant(json_char ch, const json_string & value_t, const size_t pos) json_nothrow;
    void parse(void) json_nothrow;
    json_string buffer;
    json_stream_callback_t call;
};

#endif

#endif

