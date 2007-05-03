# $Id$
.POSIX:
.PHONY: clean install dist distclean
PREFIX=/usr/local
REV=0

cxacru-info: src/cxacru-info.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

all: cxacru-info dist

clean:
	rm -f cxacru-info

distclean: clean
	rm -f cxacru-info_r*.tar.bz2
	rm -rf .tmp

install: cxacru-info
	mkdir -p $(PREFIX)/bin/
	cp cxacru-info $(PREFIX)/bin/
	mkdir -p $(PREFIX)/man/man1/
	cp doc/cxacru-info.1 $(PREFIX)/man/man1/

dist:
	rm -f "cxacru-info_r$(REV).tar.bz2"
	rm -rf .tmp
	mkdir -p ".tmp/cxacru-info_r$(REV)/" \
".tmp/cxacru-info_r$(REV)/src/" \
".tmp/cxacru-info_r$(REV)/doc/" \
".tmp/cxacru-info_r$(REV)/pkg/"
	cp -a Makefile src/ doc/ pkg/ GPL-2 ".tmp/cxacru-info_r$(REV)/"
	tar -jf "cxacru-info_r$(REV).tar.bz2" \
--numeric-owner --owner=0 --group=0 --exclude=.svn \
-C .tmp/ -c "cxacru-info_r$(REV)/"
	tar -tvjf "cxacru-info_r$(REV).tar.bz2"
	rm -rf .tmp
