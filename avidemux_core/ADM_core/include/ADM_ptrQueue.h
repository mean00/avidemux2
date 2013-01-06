/***************************************************************************
              
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_PTRQUEUE_H
#define ADM_PTRQUEUE_H

template <class T>
class ADM_ptrQueue
{
    
typedef struct queueElem
{
  queueElem   *next;
  void        *data;
}queueElem;

  protected:
    queueElem *head;
    queueElem *tail;
  public:
            ADM_ptrQueue()
            {
                head=NULL;
                tail=NULL;
            }
            ~ADM_ptrQueue()
            {
                 if(head)
                {
                  ADM_warning(">>>>>>>>Warning queue is not empty\n<<<<<<<"); 
                }
            }
    bool isEmpty(void)
    {
          if(head) return 0;
          ADM_assert(!tail);
          return 1; 
    }
    bool push(T *data)
    {
        queueElem *elem=new queueElem;

        elem->next=NULL;
        elem->data=data;
        if(!head)
        {
          head=tail=elem; 
          return 1;
        }
        ADM_assert(tail);
        tail->next=elem;
        tail=elem;
        return 1;
      }        
    bool pushBack(T *data)
    {
        queueElem *elem=new queueElem;
        elem->next=head;
        elem->data=data;
        if(!head) tail=elem;
        head=elem;
        return 1;
    }
    T *pop()
    {
        T* r=NULL;
         ADM_assert(head);         
         if(isEmpty()) return NULL;
         r=(T*)head->data;
         queueElem *tmp=head;
         head=head->next;
         if(!head)
         {
              head=tail=NULL; 
         }
         delete tmp;
         return r;
    }
  
};



#endif
