/***************************************************************************
 winetmppath.cpp  -  description
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

#include <windows.h>
#include <stdio.h>
#include "winetmppath.h"
#include "strnew.h"

extern "C" void wine_tmp_path (AVS_PIPES *avsp, int num)
{
  WCHAR path[MAX_PATH];
  GetTempPathW(sizeof(path), path);
  LPSTR (*wine_get_unix_file_name_ptr)(LPCWSTR) = NULL;
  wine_get_unix_file_name_ptr = (char *(__cdecl *)(const unsigned short *))
      GetProcAddress(GetModuleHandle("KERNEL32"),
                     "wine_get_unix_file_name");
  if (wine_get_unix_file_name_ptr == NULL) {
    fprintf(stderr, "cannot get the address of "
            "'wine_get_unix_file_name'\n");
    return;
  }

  WCHAR *prefix[] = {L"plr", L"plw", L"pfw"};
  char *unix_name;
  int i;

  unix_name = wine_get_unix_file_name_ptr(path);
  printf("%s\n", unix_name);
  //fflush(stdout);

  for (i = 0; i < 3; i++)
  {
    WCHAR fname[MAX_PATH];
    char cname[MAX_PATH];

    if (GetTempFileNameW( path, prefix[i], 0, fname))
    {
      unix_name = wine_get_unix_file_name_ptr(fname);
      printf("%s\n", unix_name);
      WideCharToMultiByte(CP_ACP, 0, fname, -1, cname, MAX_PATH, NULL, NULL);
      avsp[i].pipename = strnew(cname);
    }
    else
      printf("Error generate temp filename\n");
  }

  //fflush(stdout);
  return;
}
