.POSIX:
.PHONY: clean install
PREFIX=/usr/local

cxacru-info: cxacru-info.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f cxacru-info

install: cxacru-info
	mkdir -p $(PREFIX)/bin/
	cp cxacru-info $(PREFIX)/bin/
	mkdir -p $(PREFIX)/man/man1/
	cp cxacru-info.1 $(PREFIX)/man/man1/
