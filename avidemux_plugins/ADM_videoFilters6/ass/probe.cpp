#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "ft2build.h"
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>


#define CHECK(x) if((x)) {printf( #x " failed \n");return -1;}

/**

*/
int main(int a, char **b)
{
  FT_Library  alibrary ;
  int maj,min,patch;
  CHECK(FT_Init_FreeType(   &alibrary ));
  FT_Library_Version(   alibrary,&maj,&min,&patch );

 // build with FT 17.1.11
  printf("Freetype version %d.%d.%d\n",maj,min,patch);

  CHECK(!FcInit());

  int fcVersion=FcGetVersion();

    int fcMaj=fcVersion/10000;
    int fcMin=(fcVersion-fcMaj*10000)/100;

  printf("Fontconfig version %d :%d.%d\n",fcVersion,fcMaj,fcMin);
  // fontconfig >= 2.13 configuration is not backward compatible
  // special case *ubuntu 18.04LTS (fontconfig 2.12.6), avoid startup delay from building cache
  if(fcVersion>=21300 || fcVersion==21206)
    return 1;
  return 0;
}
