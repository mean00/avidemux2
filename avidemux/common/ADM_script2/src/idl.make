idls:=$(shell ls *.idl)
cs:=$(subst .idl,.c,$(idls))
%.c : %.idl
	echo processing $@
	jsapigen < $< > $@ 
all: $(cs)
