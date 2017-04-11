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
    bool clear()
    {
        queueElem *elem=head;
        while(elem)
        {
            queueElem *c=elem;
            elem=elem->next;
            delete c;
        }
        head=tail=NULL;
        return true;
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
        elem->next=NULL;
        elem->data=data;

        if(tail)
        {
            tail->next=elem;
            tail=elem;
        }else
        {
            head=tail=elem;
        }
        return true;
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
     T *popBack()
    {
         queueElem *h;
         T *r;
         if(isEmpty()) return NULL;
         ADM_assert(head);         
         ADM_assert(tail);  
         
         r=(T *)tail->data;         
         
         if(head==tail) // only one element..
         {
             delete tail;
             head=tail=NULL;
             return r;
         }
         
         h=head;
         while(h->next!=tail)
         {
             h=h->next;
             ADM_assert(h);
         }
         // h is now the one before the last
         h->next=NULL;
         delete tail;
         tail=h;
         return r;
    }
  
};



#endif
