CC=g++
OBJS= ffv1video.o ffv1rec.o ffv1audio.o ffv1color.o ffv1codec.o ffv1write.o frequencies.o
MEMCPY=../../avidemux/ADM_toolkit/ADM_memcpy.o ../../avidemux/ADM_toolkit/ADM_cpuCap.o 

IFLAGS= \
-I../../avidemux \
-I../../ \
-I../../avidemux/ADM_library      \

COPTSRT= -O3 -Wall -DMMX -mcpu=pentium -funroll-loops -fexpensive-optimizations -finline-functions

#COPTSRT= -O0 -g3


LIBS= ../../avidemux/ADM_memsupport.o ../../avidemux/ADM_codecs/libADM_codecs.a ../../avidemux/ADM_lavcodec/libavcodec.a  ../../avidemux/ADM_lavutil/libADM_lavutil.a -lm -lxvidcore -lpthread -ldl -lz

V4LDIR=/usr/src/linux/drivers/char/

COPTS=-DMMX $(IFLAGS) $(COPTSRT)  -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 

all:   ffv1rec

ffv1codec.o:	 ffv1codec.c  Makefile ffv1.h
	$(CC) $(COPTS) -o $@ $< -c -I../../avidemux/ADM_lavcodec -I../../avidemux/ADM_lavutil
%.o:	 %.c  Makefile ffv1.h
	$(CC) $(COPTS) -o $@ $< -c
ffv1rec:	$(OBJS)
	$(CC) $(COPTS) -o $@  $(OBJS) $(LIBS) $(MEMCPY)



clean:
	rm -f *.o ffv1rec
