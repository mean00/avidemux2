i586-pc-mingw32-g++ -DHAVE_CONFIG_H -I/rot2/include/ -I. -I. -I../..  -I../ADM_libraries -I../ADM_libraries/ADM_utilities -I../ADM_libraries/ADM_lavutil -I/usr/include/libxml2  -I/usr/include/malloc -I/usr/include/libxml2 -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT  -I.. -I../ADM_lavutil -IADM_utilities -I../ADM_utilities  -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g -O2 -I../.. -O1 -g3 -falign-loops=16 \
-c -o yad_dummy.o \
../ADM_videoFilter/ADM_vidYadif.cpp  -mno-cygwin -mms-bitfields -mwindows

i586-pc-mingw32-gcc -DHAVE_CONFIG_H -I/rot2/include/ -I. -I. -I../..  -I../ADM_libraries -I../ADM_libraries/ADM_utilities -I../ADM_libraries/ADM_lavutil -I/usr/include/libxml2  -I/usr/include/malloc -I/usr/include/libxml2 -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT  -I.. -I../ADM_lavutil -IADM_utilities -I../ADM_utilities  -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g -O2 -I../.. -O1 -g3 -falign-loops=16 \
-c -o yad_dummy2.o \
../ADM_videoFilter/ADM_vidYadif_asm.c  -mno-cygwin -mms-bitfields -mwindows

i586-pc-mingw32-g++ -shared yad_dummy.o yad_dummy2.o -o yadif.dll -L.. \
    -Wl,--export-all-symbols \
    -Wl,--enable-auto-import \
    -mno-cygwin -lmingw32  ../libavidemux2.dll.a
    #../avidemux2_gtk.exe
i586-pc-mingw32-strip -S yadiff.dll
#../avidemux2_gtk.exe.a \
