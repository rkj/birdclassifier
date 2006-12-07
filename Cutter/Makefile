CXXFLAGS = -I../

OBJS = cutter.o -lsndfile ../detect/Files.o\
			 ../mpglib/common.o ../mpglib/interface.o ../mpglib/layer3.o ../mpglib/tabinit.o ../mpglib/dct64_i386.o ../mpglib/decode_i386.o
all: $(OBJS)
	$(CXX) $(OBJS) -o Cutter

clean:
	rm *.o
	rm Cutter
	
