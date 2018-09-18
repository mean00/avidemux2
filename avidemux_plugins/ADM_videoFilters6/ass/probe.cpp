#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ft2build.h>
#include FT_FREETYPE_H


#define CHECK(x) if((x)) {printf( #x " failed \n");return -1;}

/**

*/
int main(int a, char **b)
{
  FT_Library  alibrary ;
  int maj,min,patch;
  CHECK(FT_Init_FreeType(   &alibrary ));
  FT_Library_Version(   alibrary,&maj,&min,&patch );

  printf("Freetype version %d.%d.%d\n",maj,min,patch);

  return 0;
}
