OBJS=adm_gen.cpp editor_gen.cpp
%_gen.cpp:%.admPyClass
	perl ../../../../cmake/admPyClass.pl  $<
all: $(OBJS)

clean:
	rm -f $(OBJS)
