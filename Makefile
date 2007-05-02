CFLAGS=-Wall -Werror
PREFIX=
cxacru-info:
clean:
	rm -f cxacru-info
install: cxacru-info
	mkdir -p $(PREFIX)/usr/local/bin/
	cp cxacru-info $(PREFIX)/usr/local/bin/
