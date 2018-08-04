BINDIR = ./bin.d
DST = ./dst.d

.PHONY: release
release: $(BINDIR) $(DST) $(OBJS) $(HEADERS)
	cd solver && make release -j3

.PHONY: debug
debug: $(BINDIR) $(DST) $(OBJS) $(HEADERS)
	cd solver && make debug

run: $(BINDIR)/$(SLV_AOUT)
	$(BINDIR)/$(SLV_AOUT)

%.d:
	mkdir -p $*.d

clean:
	rm -f  *.o
	rm -rf dst.d
	rm -rf bin.d
