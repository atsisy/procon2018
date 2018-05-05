BINDIR = ./bin.d
DST = ./dst.d
INCLUDE = ./include
OBJS = $(DST)/field.o $(DST)/main.o
HEADERS = $(INCLUDE)/types.hpp
CC = g++
LD = g++
CFLAGS = -Wall -O2 -std=c++14 -c

.PHONY: all
all: $(BINDIR) $(DST) $(OBJS) $(HEADERS)
	$(LD) $(OBJS) -o $(BINDIR)/bin

$(DST)/%.o: %.cpp
	$(CC) $*.cpp $(CFLAGS) -o $(DST)/$*.o

%.d:
	mkdir -p $*.d

clean:
	rm -f  *.o
	rm -rf dst.d
	rm -rf bin.d
