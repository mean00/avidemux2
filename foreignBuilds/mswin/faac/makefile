CFLAGS =-s -O3 -shared -I../include -Wl,--out-implib,libfaac.a
SRC=$(wildcard *.c)

libfaac.dll: $(SRC)
	gcc -o $@ $^ $(CFLAGS)