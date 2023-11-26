OBJS1=pyHelpers_gen.cpp
OBJS=adm_gen.cpp  editor_gen.cpp tools_gen.cpp pyDFInteger_gen.cpp  pyDFFloat_gen.cpp pyDFToggle_gen.cpp  pyDialogFactory_gen.cpp pyDFMenu_gen.cpp pyDFTimeStamp_gen.cpp pyDFLabel_gen.cpp pyDFText_gen.cpp GUI_gen.cpp 

%_gen.cpp:binding/%.admPyClass
	perl ../../../../cmake/admPyClass.pl  $<  $@
%_gen.cpp:binding/%.admPyFunc
	perl ../../../../cmake/admPyFunc.pl  $<  $@

all: $(OBJS) $(OBJS1)
	@if ! test -f version.txt; then echo 0 > version.txt; fi
	@echo $$(($$(cat version.txt) + 1)) > version.txt

clean:
	rm -f $(OBJS) $(OBJS1)
