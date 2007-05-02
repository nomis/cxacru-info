# $Id$
.POSIX:
.PHONY: clean install dist distclean
PREFIX=/usr/local

cxacru-info: cxacru-info.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

all: cxacru-info dist

clean:
	rm -f cxacru-info

distclean: clean
	rm -f cxacru-info.tar.gz

install: cxacru-info
	mkdir -p $(PREFIX)/bin/
	cp cxacru-info $(PREFIX)/bin/
	mkdir -p $(PREFIX)/man/man1/
	cp cxacru-info.1 $(PREFIX)/man/man1/

dist:
	rm -f cxacru-info.tar.gz
	tar -zf cxacru-info.tar.gz --numeric-owner --owner=0 --group=0 -c Makefile cxacru-info.c cxacru-info.1
