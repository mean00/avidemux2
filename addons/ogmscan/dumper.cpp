#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <inttypes.h>

#include "ADM_oghead.h"
OG_Header header;
int main(int n, char **c)
{
FILE *fd;

	fd=fopen(c[1],"rb");
	if(!fd) 
	{
		if(c[1])
			printf("Cannot open file:%s\n",c[1]);
		exit(-1);
	}
	while(1)
	{
		if(feof(fd)) exit(0);
		fread(&header,1,sizeof(header),fd);
		if(memcmp(&header,"OggS",4))
		{
			printf("Sync failed\n");
			exit(1);
		}
		// dump stuff
		if(!(header.header_type&1)) printf("Fresh ");
		printf(" abs pos %08x ",*(uint64_t *)(header.abs_pos));
		printf(" serial %04x ",*(uint32_t *)header.serial);
		printf(" Page seq %04x ",*(uint32_t *)header.page_sequence);
		printf(" num seg %04x ",header.nb_segment);

		if(header.header_type&2) printf("** First page** ");
		if(header.header_type&4) printf("** Last page** ");
		
		printf("\n");
		uint8_t u;
		uint32_t size=0;
		for(uint32_t i=0;i<header.nb_segment;i++)
		{
			fread(&u,1,1,fd);
			size+=u;

		}
		fseek(fd,size,SEEK_CUR);
	}


}
