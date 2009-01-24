#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
extern unsigned char *mpeg2enc_buffer;
void ad_putchar(unsigned int outbfr)
{
 	*(mpeg2enc_buffer++)=(unsigned char )outbfr;
}
#define MAX_ALLOC 1000
static int nb_alloc=0;
static char *allocated[MAX_ALLOC];

/*
	Wrapper for malloc that allocates pbuffers aligned to the
	specified byte boundary and checks for failure.
	N.b.  don't try to free the resulting pointers, eh...
*/
void *bufalloc( size_t size )
{
	void *buf=NULL;	
	if(!size) return NULL;
	
	allocated[nb_alloc]=new char[size];
	buf=(void *)allocated[nb_alloc];
	nb_alloc++;
	assert(nb_alloc<MAX_ALLOC);
	return buf;
}

void mpeg_freebuffers( void )
{
	for(int i=0;i<nb_alloc;i++)
	{
		delete [] allocated[i];
		allocated[i]=NULL;
	}
	nb_alloc=0;

}
