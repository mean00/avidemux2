/** *************************************************************************
    \fn ADM_threads.h
    \brief Handle thread synchronization functions (mutex...)
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

#ifndef ADM_THREADS_H
#define ADM_THREADS_H

#include <pthread.h>

class admMutex
{
  private:
    uint8_t       _locked;
    const char    *_name;
  public: 
    pthread_mutex_t _tex;       
    admMutex( const char *name=NULL);
    ~admMutex();        
    uint8_t lock(void);
    uint8_t unlock(void);
    uint8_t isLocked(void);
};

class admCond
{
  private:
    pthread_cond_t  _cond;
    admMutex        *_condtex;

  public:        
    uint8_t         waiting;
    uint8_t         aborted;
    admCond( admMutex *tex);
    ~admCond();        
    uint8_t wait(void);
    uint8_t wakeup(void);
    uint8_t iswaiting(void);
    uint8_t abort(void);
                
};
extern admMutex sharedMutex;
#endif
