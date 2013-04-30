PROJECT=dll

BIN_PATH=/mnt/d/usr/lang/msvc60/Bin/
CC_WIN32=cl.exe
LINK_WIN32=link.exe
WINE=/usr/bin/wine
AVISYNTH_SRCPATH=/mnt/d/src/avisynth
INCLUDE_PATH=/mnt/d/usr/lang/msvc60/Include
LIB_PATH=/mnt/d/usr/lang/msvc60/Lib
CC=$(WINE) $(BIN_PATH)$(CC_WIN32)
LINK=$(WINE) $(BIN_PATH)$(LINK_WIN32)
PIPEDLL=pipe_source
AVSLOADER=avsload
all: pipe_source.dll avsload.exe

clean:
	rm -f *.obj
	rm -f $(AVSLOADER).exe
	rm -f $(PIPEDLL).dll
	rm -f $(PIPEDLL).lib $(PIPEDLL).exp

%.obj: %.cpp
	$(CC) -GX -c $< -DDEBUGMSG -DOS_WIN32 -o $@ -I$(INCLUDE_PATH) -I$(AVISYNTH_SRCPATH)/src/core -I$(AVISYNTH_SRCPATH)/src

%.obj: %.c
	$(CC) -c $< -DDEBUGMSG -DOS_WIN32 -o $@ -I$(INCLUDE_PATH) -I$(AVISYNTH_SRCPATH)/src/core -I$(AVISYNTH_SRCPATH)/src

$(PIPEDLL).dll: $(PIPEDLL).def $(PIPEDLL).obj avspipecomm.obj cdebug.obj
	$(LINK) /LIBPATH:$(LIB_PATH)  -dll -base:0x1C000000 -out:$@ -def:$^

$(AVSLOADER).exe: $(AVSLOADER).obj avspipecomm.obj winetmppath.obj strnew.obj cdebug.obj
	$(LINK) /LIBPATH:$(LIB_PATH) -out:$(AVSLOADER).exe $^

