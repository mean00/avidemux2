#include "JSONNode.h"

#ifdef JSON_MUTEX_CALLBACKS

json_mutex_callback_t json_lock_callback(0);
json_mutex_callback_t json_unlock_callback(0);
void * global_mutex(0);
void * manager_mutex(0);

struct AutoLock {
    AutoLock(void) json_nothrow {
	   json_lock_callback(manager_mutex);
    }
    ~AutoLock(void) json_nothrow {
	   json_unlock_callback(manager_mutex);
    }
};

#include <map>
#ifdef JSON_MUTEX_MANAGE
    json_mutex_callback_t json_destroy = 0;
    std::map<void *, unsigned int> mutex_manager;

    //make sure that the global mutex is taken care of too
    struct auto_global {
	   auto_global(void) json_nothrow {}
	   ~auto_global(void) json_nothrow {
		  if (global_mutex){
			 JSON_ASSERT_SAFE(json_destroy != 0, JSON_TEXT("No json_destroy in mutex managed mode"), return;);
			 json_destroy(global_mutex);
		  }
	   }
    };
    auto_global cleanupGlobal;
#endif

void JSONNode::register_mutex_callbacks(json_mutex_callback_t lock, json_mutex_callback_t unlock, void * manager_lock) json_nothrow {
    json_lock_callback = lock;
    json_unlock_callback = unlock;
    manager_mutex = manager_lock;
}

void JSONNode::set_global_mutex(void * mutex) json_nothrow {
    global_mutex = mutex;
}

void JSONNode::set_mutex(void * mutex) json_nothrow {
    makeUniqueInternal();
    internal -> _set_mutex(mutex);
}

std::map<int, std::map<void *, unsigned int> > threadlocks;

void * JSONNode::getThisLock(JSONNode * pthis) json_nothrow {
    if (pthis -> internal -> mylock != 0){
	   return pthis -> internal -> mylock;
    }
    JSON_ASSERT(global_mutex != 0, JSON_TEXT("No global_mutex"));  //this is safe, because it's just goingi to return 0 anyway
    return global_mutex;
}

void JSONNode::lock(int thread) json_nothrow {
    JSON_ASSERT_SAFE(json_lock_callback != 0, JSON_TEXT("No locking callback"), return;);

    AutoLock lockControl;

    //first, figure out what needs to be locked
    void * thislock = getThisLock(this);
    #ifdef JSON_SAFE
	   if (json_unlikely(thislock == 0)) return;
    #endif

    //make sure that the same thread isn't locking it more than once (possible due to complex ref counting)
    std::map<int, std::map<void *, unsigned int> >::iterator it = threadlocks.find(thread);
    if (it == threadlocks.end()){
	   std::map<void *, unsigned int> newthread;
	   newthread[thislock] = 1;
	   threadlocks.insert(std::pair<int, std::map<void *, unsigned int> >(thread, newthread));
    } else {  //this thread already has some things locked, check if the current mutex is
	   std::map<void *, unsigned int> & newthread = it -> second;
	   std::map<void *, unsigned int>::iterator locker(newthread.find(thislock));
	   if (locker == newthread.end()){  //current mutex is not locked, set it to locked
		  newthread.insert(std::pair<void *, unsigned int>(thislock, 1));
	   } else {  //it's already locked, don't relock it
		  ++(locker -> second);
		  return;  //don't try to relock, it will deadlock the program
	   }
    }

    //if I need to, lock it
    json_lock_callback(thislock);
}

void JSONNode::unlock(int thread) json_nothrow{
    JSON_ASSERT_SAFE(json_unlock_callback != 0, JSON_TEXT("No unlocking callback"), return;);

    AutoLock lockControl;

    //first, figure out what needs to be locked
    void * thislock = getThisLock(this);
    #ifdef JSON_SAFE
	   if (thislock == 0) return;
    #endif

    //get it out of the map
    std::map<int, std::map<void *, unsigned int> >::iterator it = threadlocks.find(thread);
    JSON_ASSERT_SAFE(it != threadlocks.end(), JSON_TEXT("thread unlocking something it didn't lock"), return;);

    //get the mutex out of the thread
    std::map<void *, unsigned int> & newthread = it -> second;
    std::map<void *, unsigned int>::iterator locker = newthread.find(thislock);
    JSON_ASSERT_SAFE(locker != newthread.end(), JSON_TEXT("thread unlocking mutex it didn't lock"), return;);

    //unlock it
    if (--(locker -> second)) return;  //other nodes is this same thread still have a lock on it

    //if I need to, unlock it
    newthread.erase(locker);
    json_unlock_callback(thislock);
}

#ifdef JSON_MUTEX_MANAGE
    void JSONNode::register_mutex_destructor(json_mutex_callback_t destroy) json_nothrow {
	   json_destroy = destroy;
    }
#endif


void internalJSONNode::_set_mutex(void * mutex, bool unset) json_nothrow {
    if (unset) _unset_mutex();  //for reference counting
    mylock = mutex;
    if (mutex != 0){
	   #ifdef JSON_MUTEX_MANAGE
		  std::map<void *, unsigned int>::iterator it = mutex_manager.find(mutex);
		  if (it == mutex_manager.end()){
			 mutex_manager.insert(std::pair<void *, unsigned int>(mutex, 1));
		  } else {
			 ++it -> second;
		  }
	   #endif
	   if (isContainer()){
		  json_foreach(CHILDREN, myrunner){
			 (*myrunner) -> set_mutex(mutex);
		  }
	   }
    }
}

void internalJSONNode::_unset_mutex(void) json_nothrow {
    #ifdef JSON_MUTEX_MANAGE
	   if (mylock != 0){
		  std::map<void *, unsigned int>::iterator it = mutex_manager.find(mylock);
		  JSON_ASSERT_SAFE(it != mutex_manager.end(), JSON_TEXT("Mutex not managed"), return;);
		  --it -> second;
		  if (it -> second == 0){
			 JSON_ASSERT_SAFE(json_destroy, JSON_TEXT("You didn't register a destructor for mutexes"), return;);
			 mutex_manager.erase(it);
		  }
	   }
    #endif
}

#ifdef JSON_DEBUG
    #ifndef JSON_LIBRARY
	   JSONNode internalJSONNode::DumpMutex(void) const json_nothrow {
		  JSONNode mut(JSON_NODE);
		  mut.set_name(JSON_TEXT("mylock"));
		  #ifdef JSON_MUTEX_MANAGE
			 if (mylock != 0){
				mut.push_back(JSON_NEW(JSONNode(JSON_TEXT("this"), (long)mylock)));
				std::map<void *, unsigned int>::iterator it = mutex_manager.find(mylock);
				if (it == mutex_manager.end()){
				    mut.push_back(JSON_NEW(JSONNode(JSON_TEXT("references"), JSON_TEXT("error"))));
				} else {
				    mut.push_back(JSON_NEW(JSONNode(JSON_TEXT("references"), it -> second)));
				}
			 } else {
				mut = (long)mylock;
			 }
		  #else
			 mut = (long)mylock;
		  #endif
		  return mut;
	   }
    #endif
#endif

#else
    #ifdef JSON_MUTEX_MANAGE
	   #error You can not have JSON_MUTEX_MANAGE on without JSON_MUTEX_CALLBACKS
    #endif
#endif
