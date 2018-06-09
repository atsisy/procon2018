BINDIR = ./bin.d
DST = ./dst.d
INCLUDE = ./include
OBJS = $(DST)/field.o $(DST)/agent.o $(DST)/load_qr_format.o $(DST)/main.o
HEADERS = $(INCLUDE)/types.hpp
CC = g++
LD = g++
CFLAGS = -Wall -O2 -std=c++17 -c
DEBUG = -g
AOUT = bin

.PHONY: release
release: all

.PHONY: debug
debug: CFLAGS+=$(DEBUG)
debug: all

.PHONY: all
all: $(BINDIR) $(DST) $(OBJS) $(HEADERS)
	$(LD) $(OBJS) -o $(BINDIR)/$(AOUT)

run: $(BINDIR)/$(AOUT)
	$(BINDIR)/$(AOUT)

$(DST)/%.o: %.cpp
	$(CC) $*.cpp $(CFLAGS) -o $(DST)/$*.o

%.d:
	mkdir -p $*.d

clean:
	rm -f  *.o
	rm -rf dst.d
	rm -rf bin.d
