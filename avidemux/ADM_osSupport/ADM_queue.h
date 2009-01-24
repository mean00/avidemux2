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
#ifndef ADM_QUEUE_H
#define ADM_QUEUE_H

typedef struct queueElem
{
  queueElem   *next;
  void        *data;
}queueElem;

class ADM_queue
{
  protected:
    queueElem *head;
    queueElem *tail;
  public:
            ADM_queue();
            ~ADM_queue();
    uint8_t isEmpty(void);
    uint8_t push(void *data);
    uint8_t pushBack(void *data);
    uint8_t pop(void **data);
  
};



#endif
