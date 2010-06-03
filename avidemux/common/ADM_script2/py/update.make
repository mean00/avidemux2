OBJS=adm_gen.cpp  editor_gen.cpp  pyDFInteger_gen.cpp  pyDFToggle_gen.cpp  pyDialogFactory_gen.cpp pyDFMenu_gen.cpp
%_gen.cpp:%.admPyClass
	perl ../../../../cmake/admPyClass.pl  $<
all: $(OBJS)

clean:
	rm -f $(OBJS)
