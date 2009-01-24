i586-mingw32msvc-g++ -DHAVE_CONFIG_H -I/rot2/include/ -I. -I. -I../..  -I../ADM_libraries -I../ADM_libraries/ADM_utilities -I../ADM_libraries/ADM_lavutil -I/usr/include/libxml2  -I/usr/include/malloc -I/usr/include/libxml2 -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT  -I.. -I../ADM_lavutil -IADM_utilities -I../ADM_utilities  -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -g -O2 -I../.. -O1 -g3 -falign-loops=16 \
-c -o dummy.o \
ADM_vidDummyFilter.cpp  -mno-cygwin -mms-bitfields -mwindows

i586-mingw32msvc-g++ -shared dummy.o -o dummy.dll -L.. \
    -Wl,--export-all-symbols \
    -Wl,--enable-auto-import \
    -mno-cygwin -lmingw32  ../libavidemux2.dll.a
    #../avidemux2_gtk.exe
#../avidemux2_gtk.exe.a \
