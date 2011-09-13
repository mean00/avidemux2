CFLAGS =-s -O3 -shared -DHAVE_MEMCPY=1 -DHAVE_STRING_H=1 -DHAVE_STDINT_H=1 -I../include -I"." -Wl,--out-implib,libfaad.a
SRC=$(wildcard *.c)

libfaad2.dll: $(SRC)
	gcc -o $@ $^ $(CFLAGS)