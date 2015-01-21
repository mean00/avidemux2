/** *************************************************************************
    \fn ADM_threads.h
    \brief Handle thread synchronization functions (mutex...)
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

#ifndef ADM_THREADS_H
#define ADM_THREADS_H

#include "ADM_core6_export.h"
#include <pthread.h>

class ADM_CORE6_EXPORT admMutex
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

class ADM_CORE6_EXPORT admCond
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


class ADM_CORE6_EXPORT admScopedMutex
{
  private:
    admMutex       *_tex;
    
  public: 
    admScopedMutex( admMutex *tex);
    ~admScopedMutex();        
    uint8_t lock(void);
    uint8_t unlock(void);
    uint8_t isLocked(void);
};

#endif
