OBJS1=pyHelpers_gen.cpp
OBJS=adm_gen.cpp  editor_gen.cpp  pyDFInteger_gen.cpp  pyDFToggle_gen.cpp  pyDialogFactory_gen.cpp pyDFMenu_gen.cpp GUI_gen.cpp

%_gen.cpp:binding/%.admPyClass
	perl ../../../../cmake/admPyClass.pl  $<  $@
%_gen.cpp:binding/%.admPyFunc
	perl ../../../../cmake/admPyFunc.pl  $<  $@

all: $(OBJS) $(OBJS1)

clean:
	rm -f $(OBJS)
