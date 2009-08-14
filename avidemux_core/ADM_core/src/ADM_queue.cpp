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
#include "ADM_default.h"
#include "ADM_queue.h"

ADM_queue::ADM_queue()
{
  head=NULL;
  tail=NULL;
}
ADM_queue::~ADM_queue()
{
  if(head)
  {
    printf(">>>>>>>>Warning queue is not empty\n<<<<<<<"); 
  }
}
uint8_t ADM_queue::isEmpty(void)
{
  if(head) return 0;
  ADM_assert(!tail);
  return 1; 
}
uint8_t ADM_queue::push(void *data)
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
uint8_t ADM_queue::pushBack(void *data)
{
  queueElem *elem=new queueElem;

  elem->next=head;
  elem->data=data;
  if(!head) tail=elem;
  head=elem;
  return 1;
}
uint8_t ADM_queue::pop(void **data)
{
  ADM_assert(head);
  *data=NULL;
  if(isEmpty()) return 0;
  *data=head->data;
  queueElem *tmp=head;
  head=head->next;
  if(!head)
  {
    head=tail=NULL; 
  }
  delete tmp;
  return 1;
}
//EOF 
