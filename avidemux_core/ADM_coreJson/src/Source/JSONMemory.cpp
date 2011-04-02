#include "JSONMemory.h"

#ifdef JSON_MEMORY_MANAGE
    #include "JSONNode.h"
    void auto_expand::purge(void) json_nothrow {
	   for(std::map<void *, void *>::iterator i = mymap.begin(), en = mymap.end(); i != en; ++i){
		  #if defined(JSON_DEBUG) || defined(JSON_SAFE)
			 void * temp = (void*)i -> first;  //because its pass by reference
			 libjson_free<void>(temp);
		  #else
			 libjson_free<void>((void*)i -> first);
		  #endif
	   }
    }

    void auto_expand_node::purge(void) json_nothrow {
	   for(std::map<void *, JSONNode *>::iterator i = mymap.begin(), en = mymap.end(); i != en; ++i){
		  JSONNode::deleteJSONNode((JSONNode *)i -> second);
	   }
    }

    #ifdef JSON_STREAM
	   #include "JSONStream.h"
	   void auto_expand_stream::purge(void) json_nothrow {
		  for(std::map<void *, JSONStream *>::iterator i = mymap.begin(), en = mymap.end(); i != en; ++i){
			 JSONStream::deleteJSONStream((JSONStream *)i -> second);
		  }
	   }
    #endif
#endif

#ifdef JSON_MEMORY_CALLBACKS

json_malloc_t mymalloc = 0;
json_realloc_t myrealloc = 0;
json_free_t myfree = 0;

void * JSONMemory::json_malloc(size_t siz) json_nothrow {
    if (mymalloc != 0){
	   #ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
		  void * result = mymalloc(siz);
		  JSON_ASSERT(result, JSON_TEXT("out of memory"));
		  return result;
	   #else
		  return mymalloc(siz);
	   #endif
    }
    #ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
	   void * result = malloc(siz);
	   JSON_ASSERT(result, JSON_TEXT("out of memory"));
	   return result;
    #else
	   return malloc(siz);
    #endif
}

void * JSONMemory::json_realloc(void * ptr, size_t siz) json_nothrow {
    if (myrealloc != 0){
	   #ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
		  void * result = myrealloc(ptr, siz);
		  JSON_ASSERT(result, JSON_TEXT("out of memory"));
		  return result;
	   #else
		  return myrealloc(ptr, siz);
	   #endif
    }
    #ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
	   void * result = realloc(ptr, siz);
	   JSON_ASSERT(result, JSON_TEXT("out of memory"));
	   return result;
    #else
	   return realloc(ptr, siz);
    #endif
}

void JSONMemory::json_free(void * ptr) json_nothrow {
    if (myfree != 0){
	   myfree(ptr);
    } else {
	   free(ptr);
    }
}

void JSONMemory::registerMemoryCallbacks(json_malloc_t mal, json_realloc_t real, json_free_t fre) json_nothrow {
    mymalloc = mal;
    myrealloc = real;
    myfree = fre;
}

#endif
