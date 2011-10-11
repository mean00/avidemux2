CC = gcc
CCC = g++
LD = g++

CFLAGS +=  -Wall -Wno-format
OS_CFLAGS = -D_X86_=1 -DXP_WIN -DXP_WIN32 -DWIN32 -D_WINDOWS -D_WIN32 -DWINVER=0x500 -D_WIN32_WINNT=0x500 -D_MINGW -DEXPORT_JS_API

XMKSHLIBOPTS += -Wl,--out-implib=$(OBJDIR)/libjs.dll.a
RANLIB = ranlib
MKSHLIB = $(LD) -shared $(XMKSHLIBOPTS)

ifdef BUILD_OPT
OS_CFLAGS += -O2 -s
endif

#.c.o:
#      $(CC) -c -MD $*.d $(CFLAGS) $<

CPU_ARCH = x86
GFX_ARCH = win32

JSDLL_CFLAGS = -DEXPORT_JS_API
OS_LIBS = -lm -lc

PREBUILT_CPUCFG = 1
SO_SUFFIX=dll