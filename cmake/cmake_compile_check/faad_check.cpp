#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "faad.h"
int main(int argc, char **argv)
{
#ifdef OLD_FAAD_PROTO
unsigned long int srate;
#else
uint32_t srate;
#endif
uint32_t l;
uint8_t *d;
unsigned char chan;
void    *_instance;
        faacDecInit2(_instance, d,l, &srate,&chan);
        return 0;
}
