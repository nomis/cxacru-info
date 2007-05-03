.POSIX:
# $Id$
VER=0.5

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/man
.PHONY: clean install dist distclean

cxacru-info: src/cxacru-info.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

all: cxacru-info dist

clean:
	rm -f cxacru-info

distclean: clean
	rm -f cxacru-info_*.tar.bz2
	rm -rf .tmp

install: cxacru-info
	mkdir -p $(PREFIX)/bin/
	install -m 755 -d $(BINDIR)/
	install -m 755 -d $(MANDIR)/man1/
	install -m 755 cxacru-info $(BINDIR)/cxacru-info
	install -m 644 doc/cxacru-info.1 $(MANDIR)/man1/

dist:
	rm -f "cxacru-info_$(VER).tar.bz2"
	rm -rf .tmp
	mkdir -p ".tmp/cxacru-info_$(VER)/" \
".tmp/cxacru-info_$(VER)/src/" \
".tmp/cxacru-info_$(VER)/doc/" \
".tmp/cxacru-info_$(VER)/pkg/"
	cp -a Makefile src/ doc/ pkg/ GPL-2 ".tmp/cxacru-info_$(VER)/"
	tar -jf "cxacru-info_$(VER).tar.bz2" \
--numeric-owner --owner=0 --group=0 --exclude=.svn \
-C .tmp/ -c "cxacru-info_$(VER)/"
	tar -tvjf "cxacru-info_$(VER).tar.bz2"
	rm -rf .tmp
