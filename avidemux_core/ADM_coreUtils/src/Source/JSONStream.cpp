#include "JSONStream.h"

#ifdef JSON_STREAM
#include "JSONWorker.h"


JSONStream::JSONStream(json_stream_callback_t call_p) json_nothrow : call(call_p), buffer() {}

JSONStream::JSONStream(const JSONStream & orig) json_nothrow : call(orig.call), buffer(orig.buffer){}

JSONStream & JSONStream::operator =(const JSONStream & orig) json_nothrow {
    call = orig.call;
    buffer = orig.buffer;
    return *this;
}

#ifdef JSON_LIBRARY
    JSONStream & JSONStream::operator << (const json_char * str) json_nothrow {
#else
    JSONStream & JSONStream::operator << (const json_string & str) json_nothrow {
#endif
    buffer += str;
    parse();
    return *this;
}


#define QUOTECASE_STREAM()\
    case JSON_TEXT('\"'):\
	   while (*(++p) != JSON_TEXT('\"')){\
		  if (json_unlikely(*p == JSON_TEXT('\0'))) return json_string::npos;\
	   }\
	   break;


#define NULLCASE_STREAM()\
    case JSON_TEXT('\0'):\
	   return json_string::npos;\


#define BRACKET_STREAM(left, right)\
    case left: {\
	   size_t brac = 1;\
	   while (brac){\
		  switch (*(++p)){\
			 case right:\
				--brac;\
				break;\
			 case left:\
				++brac;\
				break;\
			 QUOTECASE_STREAM()\
			 NULLCASE_STREAM()\
		  }\
	   }\
	   break;}\
    case right:\
	   return json_string::npos;

size_t JSONStream::FindNextRelevant(json_char ch, const json_string & value_t, const size_t pos) json_nothrow {
    const json_char * start = value_t.c_str();
    for (const json_char * p = start + pos; *p; ++p){
	   if (json_unlikely(*p == ch)) return p - start;
	   switch (*p){
			 BRACKET_STREAM(JSON_TEXT('['), JSON_TEXT(']'))
			 BRACKET_STREAM(JSON_TEXT('{'), JSON_TEXT('}'))
			 QUOTECASE_STREAM()
	   }
    };
    return json_string::npos;
}

void JSONStream::parse(void) json_nothrow {
    size_t pos = buffer.find_first_of(JSON_TEXT("{["));
    if (json_likely(pos != json_string::npos)){
	   size_t end = (buffer[pos] == JSON_TEXT('[')) ? FindNextRelevant(JSON_TEXT(']'), buffer, pos + 1) : FindNextRelevant(JSON_TEXT('}'), buffer, pos + 1);
	   if (end != json_string::npos){
		  START_MEM_SCOPE
			 JSONNode temp(JSONWorker::parse(buffer.substr(pos, end - pos + 1)));
			 #ifndef JSON_LIBRARY
				call(temp);
			 #else
				call(&temp);
			 #endif
		  END_MEM_SCOPE
		  json_string::iterator beginning = buffer.begin();
		  buffer.erase(beginning, beginning + end);
		  parse();
	   }
    }
}

#endif
