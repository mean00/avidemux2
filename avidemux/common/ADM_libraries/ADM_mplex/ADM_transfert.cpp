//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_threads.h"
#include "ADM_transfert.h"

#define HIGH_LVL        ((TRANSFERT_BUFFER*2)/3)
#define LOW_LVL         (TRANSFERT_BUFFER/3)
// minimum amount of audio buffer we need
#define MIN_REQUIRED    (1024*1024)
//#define MPLEX_D
#define threadFailure(x) if(!(x)){printf("Condition "#x" failed at line %d\n",__LINE__);dumpStatus();ADM_assert(0);}

//**************** Transfert *******************
// *** Lot of race here : FIXME
#if 0
Transfert::Transfert(uint32_t minBuffer)
{
        cond=new admCond(&mutex);
        clientCond=new admCond(&mutex);
        _minRequired=minBuffer;
        buffer=new uint8_t[TRANSFERT_BUFFER];

        aborted=0;
        transfered_r=0;
        transfered_w=0;
        
        head=tail=0;
        
}
Transfert::~Transfert()
{
        delete cond;
        delete clientCond;
        delete [] buffer;
        
}
uint8_t Transfert::dumpStatus(void)
{
    printf("Mplex threading status : \n");
    printf("Aborted :%d\n",aborted);
    printf("head :%u\n",head);
    printf("tail :%u\n",tail);
    printf("mutex :%u\n",mutex.isLocked());
    printf("cond :%u\n",cond->iswaiting());
    printf("Clientcond :%u\n",clientCond->iswaiting());
    printf("Written : %u bytes\n",transfered_w);
    printf("Read : %u bytes\n",transfered_r);

    printf("cond waiting :%u\n",cond->waiting);
    printf("cond aborted :%u\n",cond->aborted);

    printf("clientCond waiting :%u\n",clientCond->waiting);
    printf("clientCond aborted :%u\n",clientCond->aborted);

    
    return 1;
}
uint32_t Transfert:: read(uint8_t *buf, uint32_t nb  )
{
uint32_t r=0;
uint32_t fill=0;
#ifdef MPLEX_D                 
         printf("Reading %lu\n",nb);
#endif          
        
  while(1)
  {
        mutex.lock();
        fill=tail-head;        
        if(fill>=nb)
        {
                
                memcpy(buf,buffer+head,nb);                
                head+=nb;
                r+=nb;
                transfered_r+=nb;
                goto endit;                                                    
        }
        
        // Purge
         memcpy(buf,buffer+head,fill);
         buf+=fill;
         nb-=fill;
         r+=fill;         
         head=tail=0;
         if(aborted) 
         {
                
                goto endit;
         }
         if(clientCond->iswaiting()) printf("Client : %d\n",clientCond->waiting);
         threadFailure(!clientCond->iswaiting());
         
#ifdef MPLEX_D      
         printf("Wanted : %lu , left :%lu\n",nb,fill);   
         printf("Slave sleeping\n");
#endif         
         
         cond->wait();         
         
         if(aborted) 
         {         
                mutex.lock();       
                goto endit;
         }                                   
  }
endit: 
        if(clientCond->iswaiting()) // No need to protect as the client is locked
        {
                fill=tail-head;       
                if(fill<LOW_LVL)
                {
                        printf("Waking..\n");
                        clientCond->wakeup();        
                }
        }
        mutex.unlock();
        transfered_w+=r;
        return r;               
}
//*********************************
uint8_t Transfert::fillingUp( void)
{
uint8_t r=0;

        mutex.lock();
        if((tail-head)>HIGH_LVL)
                r=1;
        else r=0;
        mutex.unlock();
        return r;
        

}
uint8_t Transfert:: write(uint8_t *buf, uint32_t nb  )
{
        if(aborted) return 0;
        
#ifdef MPLEX_D                 
         printf("Writing %lu\n",nb);
#endif          
        mutex.lock(); 
        // Need to pack ?
        if(nb+tail>=TRANSFERT_BUFFER)
        {
                memmove(buffer,buffer+head,tail-head);
                tail-=head;
                head=0;
        }
        // Overflow ?
        if(nb+tail>=TRANSFERT_BUFFER)
        {
                printf("\n When writting %lu bytes, we overflow the existing %lu bytes\n",nb,tail-head);
                threadFailure(0);
        
        }
        memcpy(buffer+tail,buf,nb);       
        transfered_w+=nb;
        tail+=nb;
        
        if(cond->iswaiting())
        {
#ifdef MPLEX_D      
           
         printf("Slave waking\n");
#endif     
                cond->wakeup();
        }                
        mutex.unlock();
        return 1;
}        
uint8_t Transfert::needData( void )
{
  int32_t l;
  uint8_t r=0;
        mutex.lock(); 
        l=tail-head;
        threadFailure(l>=0);
        mutex.unlock();
        if(l<_minRequired) r=1;
        if(cond->iswaiting()) r=1;
//        return cond->iswaiting();
        return r;
 
 }
uint8_t Transfert::abort( void )
{
        aborted=1;
        if(cond->iswaiting())
                cond->abort();
        return 1;
 
 }
  
uint8_t Transfert::clientLock( void )
{
#ifdef MPLEX_D               
         printf("Slave sleeping (%lu)\n",tail-=head);
#endif         

        mutex.lock();
        // Redo check under same mutex lock
        if((tail-head)<HIGH_LVL)
        {
            mutex.unlock();
            return 1;
        }
        clientCond->wait();               
        return 1;

}
#endif
