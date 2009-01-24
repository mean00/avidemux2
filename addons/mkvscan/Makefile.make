CFLAGS=-g3 -I../../avidemux/ADM_inputs/ADM_matroska
LDFLAGS=-g
OBJS=ebml.o mkv_c.o mkv_tags.o
%.o:%.cpp 
	$(CXX) -o $@  $< -c $(CFLAGS)
mkv: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
clean:
	rm $(OBJS) mkv
