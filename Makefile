.PHONY: install clean

CC=cc
INSTALL=install
prefix=/usr/local
bindir=$(prefix)/bin

ptxt: ptxt.c
	$(CC) -o $@ $<

install: ptxt
	$(INSTALL) -d $(bindir)
	$(INSTALL) ptxt $(bindir)/

clean:
	rm -f ptxt
