OBJS = $(DST)/field.o $(DST)/agent.o $(DST)/legacy_search.o $(DST)/load_qr_format.o $(DST)/main.o
SLV_AOUT = bin
DST = ../dst.d
INCLUDE = ../include
CC = g++
LD = g++
CFLAGS = -Wall -std=c++17 -c
OPTIMIZE =  -O2 -march=native
BINDIR = ../bin.d
DEBUG = -g

.PHONY: release
release: CFLAGS+=$(OPTIMIZE)
release: full

.PHONY: debug
debug: CFLAGS+=$(DEBUG)
debug: full

.PHONY: full
full: $(BINDIR) $(OBJS) $(HEADERS)
	$(LD) $(OBJS) -o $(BINDIR)/$(SLV_AOUT)

$(DST)/%.o: %.cpp
	$(CC) $*.cpp $(CFLAGS) -I$(INCLUDE) -o $(DST)/$*.o
