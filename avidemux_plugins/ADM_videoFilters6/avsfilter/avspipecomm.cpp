/***************************************************************************
 avspipecomm.cpp  -  description
 -------------------
 begin                : 28-04-2008
 copyright            : (C) 2008 by fahr
 email                : fahr at inbox dot ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MINGW32__
#include <unistd.h>
#include <stdint.h> // only for uint32_t !!!
#else
#include <io.h>
#include <windows.h>
#define uint32_t DWORD
#endif

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "avspipecomm.h"
#include "cdebug.h"


bool pipe_test(int hr, int hw)
{
  uint32_t test_r1 = 0;
  int sz1;

  sz1 = read(hr, &test_r1, sizeof(uint32_t));

  if (sz1 != sizeof(uint32_t))
  {
    DEBUG_PRINTF("error! read %d, errno %d\n", sz1, errno);
    return false;
  }

  sz1 = write(hw, &test_r1, sizeof(uint32_t));

  if (sz1 != sizeof(uint32_t))
  {
    DEBUG_PRINTF("error! write %d, errno %d\n", sz1, errno);
    return false;
  }

  return true;
}

#define p(X) pp##X(int h, void *data, int sz)\
{\
  int copy_sz = 0;\
  while (copy_sz != sz)\
  {\
    int isz = X(h, (char*)data + copy_sz,\
                    ((sz - copy_sz) < PIPE_MAX_TRANSFER_SZ ? (sz - copy_sz) : PIPE_MAX_TRANSFER_SZ));\
    if (isz == -1 || isz == 0) return -1;\
    copy_sz += isz;\
  }\
  return copy_sz;\
}

int p(write);
int p(read);

bool send_cmd(int hw, AVS_CMD cmd,
              const void *data, int sz)
{
  PIPE_MSG_HEADER msg = {cmd, sz};
  return ((ppwrite(hw, &msg, sizeof(msg)) == sizeof(msg)) &&
          (ppwrite(hw, data, sz) == sz));
}

bool send_cmd_with_specified_size(int hw, AVS_CMD cmd,
                                  void *data, int sz1, int sz2)
{
  PIPE_MSG_HEADER msg = {cmd, sz1 + sz2};
  return ((ppwrite(hw, &msg, sizeof(msg)) == sizeof(msg)) &&
          (ppwrite(hw, data, sz1) == sz1));
}

bool send_cmd_by_two_part(int hw, AVS_CMD cmd,
                          void *data1, int sz1,
                          void *data2, int sz2)
{
  PIPE_MSG_HEADER msg = {cmd, sz1 + sz2};
  return ((ppwrite(hw, &msg, sizeof(msg)) == sizeof(msg)) &&
          (ppwrite(hw, data1, sz1) == sz1) &&
          (ppwrite(hw, data2, sz2) == sz2));
}

bool receive_cmd(int hr, PIPE_MSG_HEADER *msg)
{
  return (ppread (hr, msg, sizeof(PIPE_MSG_HEADER)) == sizeof(PIPE_MSG_HEADER));
}

bool receive_data(int hr, PIPE_MSG_HEADER *msg,
                  void *data)
{
  return (ppread (hr, data, msg->sz) == msg->sz);
}

bool receive_data_by_size(int hr, void *data, int sz)
{
  int real_read = sz, t;
  unsigned char *buf = (unsigned char *)data;

  while (real_read && ((t = ppread (hr, buf, real_read)) != -1))
  {
    real_read -= t;
    buf += t;
  }

  if (real_read) DEBUG_PRINTF("Read %d but real read %d\n", sz, sz - real_read);
  return (!real_read);
}
