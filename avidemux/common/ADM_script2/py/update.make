OBJS=adm_gen.cpp editor_gen.cpp
%_gen.cpp:%.idl
        perl ../../../../cmake/admPyClass $%.idl
all: $(OBJS)
