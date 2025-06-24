CC 	= cc
CFLAGS  = -O2
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

prog:	winsize

docs:

winsize: winsize.o
	$(CC) -o $@ winsize.o $(LFLAGS) $(LIBS)
	$(STRIP) $@

clean:
	rm -f *.o winsize
