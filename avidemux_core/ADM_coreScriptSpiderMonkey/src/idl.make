idls:=$(shell ls *.idl)
cs:=$(subst .idl,.c,$(idls))
%.c : %.idl
	echo processing $@
	jsapigen < $< > /tmp/xx
	cat /tmp/xx   | sed 's/"\(.*\)_ignore"/"\1"/g'  | sed 's/, jjadm\(.*\)_ignore,/, jsAdm\1, /g' > $@
all: $(cs)
