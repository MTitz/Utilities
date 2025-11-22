CC 	= cc
CFLAGS  = -O3
LFLAGS	=
LIBS    =
STRIP	= strip
GROFF	= groff

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
MANDIR  = $(PREFIX)/man/man1


.c.o:
	$(CC) $(CFLAGS) -c $<


all:	prog docs

install: hdump
	cp -p hdump $(BINDIR)
	chown root:root $(BINDIR)/hdump
	chmod 755 $(BINDIR)/hdump
	cp -p hdump.1 $(MANDIR)
	chown root:root $(MANDIR)/hdump.1
	chmod 644 $(MANDIR)/hdump.1

prog:	hdump

docs:	hdump.dvi hdump.ps hdump.txt

hdump: hdump.o
	$(CC) -o $@ hdump.o $(LFLAGS) $(LIBS)
	$(STRIP) $@

hdump.dvi: hdump.1
	$(GROFF) -man -Tdvi hdump.1 > $@

hdump.ps: hdump.1
	$(GROFF) -man -Tps hdump.1 > $@

hdump.txt: hdump.1
	$(GROFF) -man -Tascii -P-u -P-b hdump.1 > $@

clean:
	rm -f *.o hdump hdump.dvi hdump.ps hdump.txt
